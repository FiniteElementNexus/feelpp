/* -*- mode: c++; coding: utf-8; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; show-trailing-whitespace: t -*- vim:fenc=utf-8:ft=cpp:et:sw=4:ts=4:sts=4

  This file is part of the Feel library

  Author(s): Christophe Prud'homme <christophe.prudhomme@feelpp.org>
       Date: 2005-11-13

  Copyright (C) 2005,2006 EPFL
  Copyright (C) 2007-2012 Universite Joseph Fourier (Grenoble I)

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 3.0 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/
/**
   \file matrixeigen.hpp
   \author Christophe Prud'homme <christophe.prudhomme@feelpp.org>
   \date 2005-11-13
 */
#ifndef __MatrixEigen_H
#define __MatrixEigen_H 1

#include <set>

#include <boost/version.hpp>
#if (BOOST_VERSION >= 103400)
#include <boost/none.hpp>
#else
#include <boost/none_t.hpp>
#endif /* BOOST_VERSION >= 103400 */

#include <Eigen/Core>
#include <unsupported/Eigen/MatrixFunctions>

#include <feel/feelalg/matrixsparse.hpp>
#include <feel/feelalg/vectorublas.hpp>

#include <feel/feelmath/jacobiellipticfunctions.hpp>


namespace Feel
{
//template<typename T> class VectorUblas;

/*!
 *
 * \brief interface to eigen sparse matrix
 *
 * this class is a wrapper around \c csr_matrix<> and \c csc_matrix<>
 * data type from \c eigen:: .
 *
 *
 * \code
 * // csr matrix
 * MatrixEigen<T,eigen::row_major> m;
 * // csc matrix
 * MatrixEigen<T,eigen::col_major> m;
 * \endcode
 *
 *  @author Christophe Prud'homme
 *  @see
 */
template<typename T>
class MatrixEigenDense : public MatrixSparse<T>
{
    typedef MatrixSparse<T> super;
public:


    /** @name Typedefs
     */
    //@{
    using size_type = uint32_type;
    typedef T value_type;
    typedef typename type_traits<value_type>::real_type real_type;
    typedef Eigen::Matrix<T,Eigen::Dynamic,Eigen::Dynamic> matrix_type;
    typedef typename super::graph_type graph_type;
    typedef typename super::graph_ptrtype graph_ptrtype;
    //@}

    /** @name Constructors, destructor
     */
    //@{

    MatrixEigenDense();

    MatrixEigenDense( size_type r, size_type c, worldcomm_ptr_t const& worldComm=Environment::worldCommPtr() );

    MatrixEigenDense( datamap_ptrtype<> const& dmRow, datamap_ptrtype<> const& dmCol );

    MatrixEigenDense( MatrixEigenDense const & m );

    ~MatrixEigenDense() override;


    //@}

    /** @name Operator overloads
     */
    //@{

    MatrixEigenDense<T> & operator = ( MatrixSparse<value_type> const& M ) override
    {
        return *this;
    }


    value_type  operator()( size_type i, size_type j ) const override
    {
        return M_mat( i, j );
    }

    using clone_ptrtype = typename super::clone_ptrtype;
    clone_ptrtype clone() const override { return std::make_shared<MatrixEigenDense<T>>( *this ); }
    //@}

    /** @name Accessors
     */
    //@{

    /**
     * @returns \p m, the row-dimension of
     * the matrix where the marix is \f$ M \times N \f$.
     */
    size_type size1 () const override
    {
        return M_mat.rows();
    }

    /**
     * @returns \p n, the column-dimension of
     * the matrix where the marix is \f$ M \times N \f$.
     */
    size_type size2 () const override
    {
        return M_mat.cols();
    }

    /**
     * \return the number of non-zeros entries in the matrix
     */
    size_type nnz() const override
    {
        return M_mat.rows()*M_mat.cols();
    }

    /**
     * return row_start, the index of the first
     * matrix row stored on this processor
     */
    size_type rowStart () const override
    {
        return 0;
    }

    /**
     * return row_stop, the index of the last
     * matrix row (+1) stored on this processor
     */
    size_type rowStop () const override
    {
        return 0;
    }

    /**
     * \return true if matrix is initialized/usable, false otherwise
     */
    bool isInitialized() const override
    {
        return M_is_initialized;
    }

    /**
     * \c close the eigen matrix, that will copy the content of write
     * optimized matrix into a read optimized matrix
     */
    void close () const override;


    /**
     * Returns the read optimized eigen matrix.
     */
    matrix_type const& mat () const
    {
        return M_mat;
    }

    /**
     * Returns the read optimized eigen matrix.
     */
    matrix_type & mat ()
    {
        return M_mat;
    }

    //@}

    /** @name  Mutators
     */
    //@{


    //@}

    /** @name  Methods
     */
    //@{

    /**
     * Initialize a Eigen matrix that is of global
     * dimension \f$ m \times  n \f$ with local dimensions
     * \f$ m_l \times n_l \f$.  \p nnz is the number of on-processor
     * nonzeros per row (defaults to 30).
     * \p noz is the number of on-processor
     * nonzeros per row (defaults to 30).
     */
    void init ( const size_type m,
                const size_type n,
                const size_type m_l,
                const size_type n_l,
                const size_type nnz=30,
                const size_type noz=10 ) override;

    /**
     * Initialize using sparsity structure computed by \p dof_map.
     */
    void init ( const size_type m,
                const size_type n,
                const size_type m_l,
                const size_type n_l,
                graph_ptrtype const& graph ) override;

    /**
     * Release all memory and return
     * to a state just like after
     * having called the default
     * constructor.
     */
    void clear () override
    {
        //eigen::resize( M_mat, 0, 0 );
        M_mat.setZero( M_mat.rows(), M_mat.cols() );
    }

    /**
     * Set all entries to 0. This method retains
     * sparsity structure.
     */
    void zero () override
    {
        M_mat.setZero( M_mat.rows(), M_mat.cols() );
    }

    void zero ( size_type start1, size_type stop1, size_type start2, size_type stop2 ) override
    {
    }

    /**
     * Add \p value to the element
     * \p (i,j).  Throws an error if
     * the entry does not
     * exist. Still, it is allowed to
     * store zero values in
     * non-existent fields.
     */
    void add ( const size_type i,
               const size_type j,
               const value_type& value ) override
    {
        M_mat( i, j ) += value;
    }

    /**
      * set \p value to the element
      * \p (i,j).  Throws an error if
      * the entry does not
      * exist. Still, it is allowed to
      * store zero values in
      * non-existent fields.
      */
    void set ( const size_type i,
               const size_type j,
               const value_type& value ) override
    {
        M_mat( i, j ) = value;
    }


    /**
     * Print the contents of the matrix in Matlab's
     * sparse matrix format. Optionally prints the
     * matrix to the file named \p name.  If \p name
     * is not specified it is dumped to the screen.
     */
    void printMatlab( const std::string name="NULL" ) const override;



    void resize( size_type nr, size_type nc, bool /*preserve*/ = false );

    /**
     * Copies the diagonal part of the matrix into \p dest.
     */
    void diagonal ( Vector<T>& dest ) const override;

    /**
     * \return \f$ v^T M u \f$
     */
    real_type
    energy( Vector<value_type> const& __v,
            Vector<value_type> const& __u, bool transpose = false ) const override;

    /**
     * eliminates row without change pattern, and put 1 on the diagonal
     * entry
     *
     *\warning if the matrix was symmetric before this operation, it
     * won't be afterwards. So use the proper solver (nonsymmetric)
     */
    void zeroRows( std::vector<int> const& rows, Vector<value_type> const& values, Vector<value_type>& rhs, Context const& on_context, value_type value_on_diagonal ) override;

    void init() {}

    /**
     * Add the full matrix to the
     * Petsc matrix.  This is useful
     * for adding an element matrix
     * at assembly time
     */
    void addMatrix( const ublas::matrix<T, ublas::row_major>&,
                    const std::vector<size_type>&,
                    const std::vector<size_type>& ) override {}

    /**
     * Same, but assumes the row and column maps are the same.
     * Thus the matrix \p dm must be square.
     */
    void addMatrix( const boost::numeric::ublas::matrix<T, ublas::row_major>&, const std::vector<size_type>& ) override {}

    /**
     * Add a Sparse matrix \p _X, scaled with \p _a, to \p this,
     * stores the result in \p this:
     * \f$\texttt{this} = \_a*\_X + \texttt{this} \f$.
     */
    void addMatrix( value_type v, MatrixSparse<value_type> const& _m, Feel::MatrixStructure matStruc = Feel::SAME_NONZERO_PATTERN ) override;


    /**
     * Multiply this by a Sparse matrix \p In,
     * stores the result in \p Res:
     * \f$ Res = \texttt{this}*In \f$.
     */
    void matMatMult ( MatrixSparse<value_type> const& In, MatrixSparse<value_type> &Res ) const override;

    /**
     * Multiply this by a Sparse matrix \p In,
     * stores the result in \p Res:
     * \f$ Res = \texttt{this}*In \f$.
     */
    void matInverse ( MatrixSparse<value_type> &Inv ) override;

    /**
     * This function creates a matrix called "submatrix" which is defined
     * by the row and column indices given in the "rows" and "cols" entries.
     * Currently this operation is only defined for the PetscMatrix type.
     */
    void createSubmatrix( MatrixSparse<T>& submatrix,
                          const std::vector<size_type>& rows,
                          const std::vector<size_type>& cols ) const override;



    /**
     * Add the full matrix to the
     * Sparse matrix.  This is useful
     * for adding an element matrix
     * at assembly time
     */
    void addMatrix ( int* rows, int nrows,
                     int* cols, int ncols,
                     value_type* data,
                     size_type K = 0,
                     size_type K2 = invalid_v<size_type>) override;

    void scale( const T a ) override;

    /**
     * Returns the transpose of a matrix
     *
     * \param M the matrix to transpose
     * \param Mt the matrix transposed
     */
    void transpose( MatrixSparse<value_type>& Mt, size_type options ) const override;

    /**
     * Return the l1-norm of the matrix, that is
     * \f$|M|_1=max_{all columns j}\sum_{all rows i} |M_ij|\f$, (max. sum of columns).
     *
     * This is the natural matrix norm that is compatible to the
     * l1-norm for vectors, i.e.  \f$|Mv|_1\leq |M|_1 |v|_1\f$.
     */
    real_type l1Norm() const override
    {
        return real_type( 0 );
    }

    /**
     * Return the linfty-norm of the matrix, that is
     *
     * \f$|M|_\infty=max_{all rows i}\sum_{all columns j} |M_ij|\f$,
     *
     * (max. sum of rows).
     * This is the natural matrix norm that is
     * compatible to the linfty-norm of vectors, i.e.
     * \f$|Mv|_\infty \leq |M|_\infty |v|_\infty\f$.
     */
    real_type linftyNorm() const override
    {
        return real_type( 0 );
    }

    /**
     * Return the square root of the matrix
     */
    void sqrt( MatrixSparse<value_type>& _m ) const override;

    std::shared_ptr<MatrixEigenDense<T>> sqrt() const;

    /**
     * Compute the eigenvalues of the current Sparse matrix,
     * stores the result in \p Eingvs:
     * \f$ Engvs = \texttt{this}*In \f$.
     */
    void eigenValues ( std::vector<std::complex<double>> &Eingvs );



    MatrixEigenDense<T>  operator * ( MatrixEigenDense<T> const& M )
    {
        MatrixEigenDense<T>  R;
        R.mat() = this->mat() * M.mat();
        return R;
    }

    MatrixEigenDense<T>  operator - ( MatrixEigenDense<T> const& M )
    {
        MatrixEigenDense<T>  R;
        R.mat() = this->mat() - M.mat();
        return R;
    }

    MatrixEigenDense<T> & operator = ( MatrixEigenDense<T> const& M )
    {
        M_mat = M.mat();
        return *this;
    }

    /**
     * update a block matrix
     */
    void updateBlockMat( std::shared_ptr<MatrixSparse<value_type> > const& m, std::vector<size_type> const& start_i, std::vector<size_type> const& start_j ) override;


    //@}

    void applyInverseSqrt( Vector<value_type>& vec_in, Vector<value_type>& vec_out ) override;



protected:

private:

    bool M_is_initialized;

    /**
     * the eigen sparse matrix data structure
     */
    mutable matrix_type M_mat;
};

#if !defined( FEELPP_INSTANTIATE_MATRIXEIGENDENSE )
extern template class MatrixEigenDense<double>;
extern template class MatrixEigenDense<std::complex<double>>;
#endif




} // Feel
#endif /* __MatrixEigenDense_H */
