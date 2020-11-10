from pyfeelpp import core,mesh,discr,models
from pyfeelpptoolboxes.modelcore import *
from _hdg import *

_hdgs={
    'hdg(2,1)':HDGPoisson_2DP1,
    'hdg(2,2)':HDGPoisson_2DP2,
    'hdg(3,1)':HDGPoisson_3DP1,
    'hdg(3,2)':HDGPoisson_3DP2,
}

def hdgpoisson( dim=2, order=1, prefix="", prefix_toolbox="hdg.poisson", worldComm=None ):
    """create a hdg toolbox solver
    Keyword arguments:
    dim -- the dimension (default: 2)
    order -- the polynomial order for the fields : potential, flux, displacement, stress and associated traces (default: 1)
    prefix -- application prefix for the HDG poisson
    worldComm -- the parallel communicator for the mesh (default: core.Environment::worldCommPtr())
    """
    if worldComm is None:
        worldComm=core.Environment.worldCommPtr()
    key='hdg('+str(dim)+','+str(order)+')'
    if worldComm.isMasterRank():
        print(key)
    if key not in _hdgs:
        raise RuntimeError('HDG solver '+key+' not existing')
    _prefix= prefix_toolbox+"."+prefix if prefix else prefix_toolbox
    return _hdgs[key]( _prefix, worldComm )
