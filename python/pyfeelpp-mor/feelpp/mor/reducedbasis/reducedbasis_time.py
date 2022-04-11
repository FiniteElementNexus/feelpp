from .reducedbasis import *
import warnings
import scipy.sparse as spsp


def taille_dict(d):
    n = 0
    for k in d:
        n += len(d[k])
    return n

class reducedbasis_time(reducedbasis):

    def __init__(self, Aq, Fp, model, mubar, alphaLB, Mr, tf, K) -> None:
        """Initialise the object

        Args:
            `Aq` (list of PETSc.Mat)       : matrices Aq given by the affine decomposition
            `Fp` (list of PETSc.Vec)       : vectors Fq given by the decomposition of right-hand side
            `model` (ToolboxMor_{2|3}D)    : model DEIM used for the decomposition
            `mubar` (ParameterSpaceElement): parameter mu_bar for the enrgy norm
            `alphaLB` (func)               : function mu ↦ alphaLB(mu)
            `Mr` (list of PETSc.Mat)       : matrices Mq of mass for the scalar product
            `tf` (float)                   : final time
            `K` (int)                      : number of time iterations
        """
        super().__init__(Aq, Fp, model, mubar, alphaLB)
        self.Mr = Mr
        self.Qm = len(Mr)
        self.MN : spsp.csc_matrix

        self.tf = tf
        self.K  = K
        self.dt = tf / K

        self.MNr : list # len Qm, each matrix of size (N,N)

        self.Fkp : dict # size K*Qf : Fkp[k,p] <-> F^{k,p}
        self.Mnr : dict # size N*Qm : Mnr[k,p] <-> M^{n,r}

        self.FF = np.zeros((K, self.Qf, self.Qf)) #       FF[k,p,p_]    = (Fkp,Fkp_)X
        self.FM : np.ndarray    # shape (K, Qf, Qm, N)    FM[k,p,r,n]   = (Fkp, Mrn)X
        self.FL : np.ndarray    # shape (K, Qf, Qa, N)    FL[k,p,q,n]   = (Lnq, Fkp)X
        self.ML : np.ndarray    # shape (Qm, N, Qa, N)    ML[r,n,q,n_]  = (Lnq, Mrn_)X
        self.MM : np.ndarray    # shape (Qm, N, Qm, N)    MM[q,n,q_,n_] = (Mrn, Mr_n_)X

        # quantities depending on mu, but stored to avoid re-computation
        self.err = np.zeros(K)          # online error
        self.DeltaN = np.zeros(K)       # error bound using offline / online computation
        self.EnNorm = np.zeros(K)       # energy norm, filled as the resolution goes


    def assembleM(self, beta):
        """Assemble the matrix from a given vector of parameters
           M = sum_r beta[r]*Mr[r]

        Args:
            beta (list): list of parameters betaM

        Returns:
            PETSc.Mat: assembled matrix
        """
        assert( len(beta) == self.Qm ), f"Number of param ({len(beta)}) should be {self.Qm}"

        M = self.Mr[0].duplicate()
        for r in range(0, self.Qm):
            M += self.Aq[r] * beta[r]
        return M

    def assembleMN(self, beta):
        """Assemble the matrix from a given vector of parameters
           M = sum_r beta[r]*Mr[r]

        Args:
            beta (list): list of parameters betaM

        Returns:
            PETSc.Mat: assembled matrix
        """
        assert( len(beta) == self.Qm ), f"Number of param ({len(beta)}) should be {self.Qm}"

        MN = spsp.csc_matrix(self.MNr[0].shape)
        for r in range(0, self.Qm):
            MN += self.MNr[r] * beta[r]
        return MN


    def generateMNr(self) -> None:
        """Generates the reduced matrices MNr
        """
        self.MNr = []
        for _ in range(self.Qm):
            self.MNr.append(np.zeros((self.N, self.N)))
        for i,u in enumerate(self.Z):
            for j,v in enumerate(self.Z):
                for r in range(self.Qm):
                    self.MNr[r][i,j] = v.dot( self.Mr[r] * u)

    def generateLN(self):
        return np.ones(self.NN) @ self.M @ self.Z

    def computeOfflineReducedBasis(self, mus, orth=True):
        """Computes the reduced basis from a set of parameters

        Args:
            mus (list of ParameterSpaceElement): list of parameters
            orth (bool, optional): orthonormalize the basis. Defaults to True.
        """
        super().computeOfflineReducedBasis(mus, orth=orth)
        self.generateMNr()

    def generateBasis(self, musk, orth=True) -> None:
        """Generates the reduced basis matrix from different parameters and different instants

        Args:
            musk (dict): dict as {mu:[k0,k1,...], ...} where ki are sorted instants
        """

        self.N = taille_dict(musk)
        self.Z = []
        ind = 0

        def g(k): return 1 if k == 0 else 0
        # def g(k): return 1 - float(np.cos(k*self.dt))

        for mu in musk:
            beta = self.model.computeBetaQm(mu)
            Amu = self.assembleA(beta[0][0])
            Fmu = self.assembleF(beta[1][0][0])
            Mmu = self.assembleM(beta[2][0])
            mat = Mmu + self.dt * Amu

            u = Fmu.duplicate()
            u.set(0)    # initial solution TODO

            mx = musk[mu][-1]
            cur = 0

            for k in range(mx):
                rhs = g(k) * self.dt * Fmu + Mmu * u
                self.ksp.setOperators(mat)
                self.ksp.setConvergenceHistory()
                sol = Fmu.duplicate()
                sol.set(0)
                self.ksp.solve(rhs, sol)
                u = sol.copy()

                if k+1 == musk[mu][cur]:
                    self.Z.append(sol.copy())
                    ind += 1
                    cur += 1

        if orth:
            self.orthonormalizeZ()
        self.generateANq()
        self.generateFNp()
        self.generateLNp()
        self.generateMNr()


    """Solve for a given parameter
    """

    def solveTime(self, mu, g):
        """Solves the time-dependatnequation for a given parameter and time-dependant function

        Args:
            mu (ParameterSpaceElement): parameter
            g (np.ndarray): g function of the right-hand side (g[k]=g(k*dt))

        Returns:
            np.ndarray: solution uN of the equation at time t = K*dt
        """
        beta = self.model.computeBetaQm(mu)
        ANmu = self.assembleAN(beta[0][0])
        FNmu = self.assembleFN(beta[1][0][0])
        MNmu = np.array(self.assembleMN(beta[2][0]))

        mat = spsp.csc_matrix(MNmu + self.dt * ANmu)
        matLu = splu(mat)
        u = np.zeros(self.N)    # initial solution

        for k in range(1, self.K):
            sol = matLu.solve(g[k] * self.dt * FNmu + MNmu @ u)
            u = sol.copy()
        return u

    def solveTimeForStudy(self, mu, g):
        beta = self.model.computeBetaQm(mu)
        ANmu = self.assembleAN(beta[0][0])
        FNmu = self.assembleFN(beta[1][0][0])
        MNmu = np.array(self.assembleMN(beta[2][0]))

        Amu = self.assembleA(beta[0][0])
        Fmu = self.assembleF(beta[1][0][0])
        Mmu = self.assembleM(beta[2][0])

        mat = spsp.csc_matrix(MNmu + self.dt * ANmu)
        matLu = splu(mat)
        bigMat = Mmu + self.dt * Amu
        self.ksp.setOperators(bigMat)

        uN = np.zeros(self.N)
        u = self.Fq[0].duplicate()

        ones = self.Fq[0].duplicate()
        ones.set(1)
        onesN = np.ones(self.N)

        t = []
        sN = []
        s = []
        sDiff = []
        normN = []
        norm = []
        normDiff = []

        tmpN = np.zeros(self.K)
        tmp = np.zeros(self.K)
        tmpDiff = np.zeros(self.K)

        # résoudre dans le cas stationnaire au lieu de la boucle en temps TODO

        for k in range(1, self.K):
            t.append(k * self.dt)

            solN = matLu.solve(g[k] * self.dt * FNmu + MNmu @ uN)
            uN = solN.copy()

            rhs = float(g[k]) * self.dt * Fmu + Mmu * u
            self.ksp.setConvergenceHistory()
            sol = self.Fq[0].duplicate()
            sol.set(0)
            self.ksp.solve(rhs, sol)
            u = sol.copy()

            sN.append(onesN @ MNmu @ uN)
            s.append(ones.dot(Mmu * u))
            sDiff.append(np.abs(s[-1] - sN[-1]))

            uplus = self.projFE(uN)
            diff = u - uplus
            
            tmpN[k] = uplus.dot( Amu * uplus )
            tmp[k] = u.dot( Amu * u )
            tmpDiff[k] = diff.dot(Amu * diff)

            normN.append( np.sqrt(uplus.dot(Mmu * uplus) + self.dt * tmpN.sum()) )
            norm.append( np.sqrt(u.dot(Mmu * u) + self.dt * tmp.sum()) )
            normDiff.append( np.sqrt(diff.dot(Mmu * diff) + self.dt * tmpDiff.sum()) )

        return t, sN, s, sDiff, normN, norm, normDiff


    """
    Error handling
    """
    def computeDirectError(self, mu, precalc=None):
        # TODO
        warnings.warn("The function computeDirectError has not yet been implemented")
        return super().computeDirectError(mu, precalc=precalc)

    def computeOfflineError(self, g):
        """Stores the offline data for error bound computation

        Args:
            g (np.ndarray): right-hand side time-dependant function
        """
        super().computeOfflineError()   # compute LL

        self.Fkp = {}
        self.Mnr = {}

        pc = self.ksp.getPC()
        pc.setType(self.PC_TYPE)
        self.ksp.setOperators(self.Abar)
        sol = self.Fq[0].duplicate()

        for k in range(self.K):
            for p in range(self.Qf):
                rhs = self.Fq[p] * g[k+1]
                self.reshist = {}
                self.ksp.solve(rhs, sol)
                self.Fkp[k, p] = sol.copy()
        
        for n, ksi in enumerate(self.Z):
            for r, Mr in enumerate(self.Mr):
                self.reshist = {}
                self.ksp.solve( Mr * ksi, sol)
                self.Mnr[n, r] = sol.copy()
            
        self.FF = np.zeros((self.K, self.Qf, self.Qf))
        self.FM = np.zeros((self.K, self.Qf, self.Qm, self.N))
        self.FL = np.zeros((self.K, self.Qf, self.Qa, self.N))
        self.ML = np.zeros((self.Qm, self.N, self.Qa, self.N))
        self.MM = np.zeros((self.Qm, self.N, self.Qm, self.N))

        for k in range(self.K):
            for p in range(self.Qf):
                for p_ in range(self.Qf):
                    self.FF[k, p, p_] = self.scalarA(self.Fkp[k,p], self.Fkp[k,p_])
            
                for n in range(self.N):
                    for r in range(self.Qm):
                        self.FM[k,p,r,n] = self.scalarA(self.Fkp[k,p], self.Mnr[n,r])
                    for q in range(self.Qa):
                        self.FL[k,p,q,n] = self.scalarA(self.Lnq[n,q], self.Fkp[k,p])
        
        for n in range(self.N):
            for r in range(self.Qm):
                for n_ in range(self.N):
                    for q in range(self.Qa):
                        self.ML[r,n,q,n_] = self.scalarA(self.Lnq[n,q], self.Mnr[n,r])
                    for r_ in range(self.Qm):
                        self.MM[r,n,r_,n_] = self.scalarA(self.Mnr[n,r], self.Mnr[n_,r_])

    def expandOffline(self):
        super().expandOffline()
        # self.Fkp and self.FF are independant of N, so they don't change
        self.FM = np.concatenate( (self.FM, np.zeros(self.K, self.Qf, self.Qm, 1)), axis=3 )
        self.FL = np.concatenate( (self.FL, np.zeros(self.K, self.Qf, self.Qa, 1)), axis=3 )
        self.ML = np.concatenate( ( self.ML, np.zeros( (self.Qm, 1, self.Qa, self.N) ) ), axis=1 )
        self.ML = np.concatenate( ( self.ML, np.zeros( (self.Qm, self.N+1, self.Qa, 1) ) ), axis=3 )
        self.MM = np.concatenate( ( self.MM, np.zeros( (self.Qm, 1, self.Qm, self.N) ) ), axis=1 )
        self.MM = np.concatenate( ( self.MM, np.zeros( (self.Qm, self.N+1, self.Qm, 1) ) ), axis=3 )

        pc = self.ksp.getPC()
        pc.setType(self.PC_TYPE)
        self.ksp.setOperators(self.Abar)

        for r, Mr in enumerate(self.Mr):
            sol = self.Fq[0].duplicate()
            self.reshist = {}
            self.ksp.solve(-Mr*self.Z[-1], sol)
            self.Mnr[self.N, r] = sol.copy()
        
        for k in range(self.K):
            for p in range(self.p):
                for r in range(self.Qm):
                    self.FM[k,p,r,-1] = self.scalarA(self.Fkp[k,p], self.Mnr[self.N,r])
                for q in range(self.Qa):
                    self.FL[k,p,q,-1] = self.scalarA(self.Lnq[self.N,q], self.Fkp[k,p])
        
        for r in range(self.Qm):
            for q in range(self.Qa):
                self.ML[r, -1, q, -1] = self.scalarA(self.Lnq[self.N,q], self.Mnr[self.N,r])
            for r_ in range(self.Qm):
                for n in range(self.N):
                    self.MM[-1,r,n,r_] = self.scalarA(self.Mnr[n,r], self.Mnr[self.N,r_])
                    self.MM[-1,r,n,r_] = self.scalarA(self.Mnr[self.N,r], self.Mnr[n,r_])


    def computeOnlineError_k(self, mu, uN, uNm1, k, precalc=None):
        """Computes the online error SQUARED, for a parameter mu

        Args:
            mu (ParameterSpaceElement): parameter
            uk (np.ndarray): solution at time k (of size N)
            ukm1 (np.ndarray): solution at time k-1
            k (int): index of current temps

        Returns:
            float: ||hat{e}^k||^2
        """
        if precalc is None:
            beta_ = self.model.computeBetaQm(mu)
            betaA = beta_[0][0]
            betaF = beta_[1][0][0]
            betaM = beta_[2]
            A_mu = self.assembleAN(betaA)
            F_mu = self.assembleFN(betaF)
            M_mu = self.assembleMN(betaM)

            uN = np.linalg.solve(A_mu, F_mu) # ?
        else:
            betaA = precalc["betaA"]
            betaF = precalc["betaF"]
            betaM = precalc["betaM"]
            uN = precalc["uN"]
            uNm1 = precalc["uNm1"]

        diff = ((uN.flatten() - uNm1.flatten()) / self.dt).T

        s1 = betaF @ self.FF[k] @ betaF
        s2 = 0
        s3 = 0
        s4 = 0
        s5 = 0
        s6 = 0

        for p in range(self.Qf):
            for n in range(self.N):
                for r in range(self.Qm):
                    s2 += betaF[p] * betaM[r] * diff[n] * self.FM[k,p,r,n]
                for q in range(self.Qa):
                    s3 += betaF[p] * betaA[q] * uN[n] * self.FL[k,p,q,n]

        for n in range(self.N):
            for n_ in range(self.N):
                for r in range(self.Qm):
                    for q in range(self.Qa):
                        s4 += diff[n] * uN[n_] * betaM[r] * betaA[q] * self.ML[r,n,q,n_]
                    for r_ in range(self.Qm):
                        s5 += betaM[r] * betaM[r_] * diff[n] * diff[n_] * self.MM[r,n,r_,n_]
                for q in range(self.Qa):
                    for q_ in range(self.Qa):
                        s6 += betaA[q] * betaA[q_] * uN[n] * uN[n_] * self.LL[q,n,q_,n_]

        return s1 + 2 * (s2 + s3 + s4) + s5 + s6


    def computeOnlineError(self, mu, g, computeEnergyNorm=False):
        """Computes online bound error

        Args:
            mu (ParameterSpaceElement): parameter
            g (np.ndarray): right-hand side time-dependant function
            computeEnergyNorm (bool): computes the energy normsuring the resolution (stroed in self.EnNorm). Dafault to False

        Returns:
            float: Δ_N^k
        """
        beta_ = self.model.computeBetaQm(mu)
        betaA = beta_[0][0]
        betaF = beta_[1][0][0]
        betaM = beta_[2][0]
        ANmu = self.assembleAN(betaA)
        FNmu = self.assembleFN(betaF)
        MNmu = self.assembleMN(betaM)
        precalc = {
            "betaA": betaA,
            "betaF": betaF,
            "betaM": betaM
        }

        matN = splu(MNmu + self.dt * spsp.csc_matrix(ANmu))
        alpm1 = 1 / np.sqrt(self.alphaLB(mu))

        aNorm = np.zeros(self.K)
        self.err[:] = 0
        self.DeltaN[:] = 0

        u = np.zeros(self.N)    # initial condition
        if computeEnergyNorm:
            self.EnNorm[:] = 0
        
        for k in tqdm(range(1, self.K+1)):
            u_tmp = matN.solve( (g[k] * self.dt * FNmu + MNmu @ u).T )#.reshape(self.N)
            precalc["uN"] = u_tmp
            precalc["uNm1"] = u
            self.err[k-1] = self.computeOnlineError_k(mu, u_tmp, u, k-1, precalc=precalc)
            u = np.array(u_tmp).flatten()

            self.DeltaN[k-1] = alpm1 * np.sqrt(self.dt * self.err[:k].sum())

            if computeEnergyNorm:
                aNorm[k-1] = u.T @ ANmu @ u
                self.EnNorm[k-1] = np.sqrt( u.T @ MNmu @ u + self.dt * aNorm.sum() )

        return self.DeltaN[-1]

