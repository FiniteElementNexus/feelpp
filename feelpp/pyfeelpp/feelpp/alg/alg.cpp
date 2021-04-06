//! -*- mode: c++; coding: utf-8; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; show-trailing-whitespace: t  -*- vim:fenc=utf-8:ft=cpp:et:sw=4:ts=4:sts=4
//!
//! This file is part of the Feel++ library
//!
//! This library is free software; you can redistribute it and/or
//! modify it under the terms of the GNU Lesser General Public
//! License as published by the Free Software Foundation; either
//! version 2.1 of the License, or (at your option) any later version.
//!
//! This library is distributed in the hope that it will be useful,
//! but WITHOUT ANY WARRANTY; without even the implied warranty of
//! MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
//! Lesser General Public License for more details.
//!
//! You should have received a copy of the GNU Lesser General Public
//! License along with this library; if not, write to the Free Software
//! Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
//!
//! @file
//! @author Christophe Prud'homme <christophe.prudhomme@feelpp.org>
//! @date 25 Jul 2018
//! @copyright 2018 Feel++ Consortium
//!
#include <pybind11/pybind11.h>

#include <feel/feelalg/vectorpetsc.hpp>
#include <feel/feelalg/matrixpetsc.hpp>
#include <mpi4py/mpi4py.h>

namespace py = pybind11;
using namespace Feel;

class PyVectorDouble : Vector<double,unsigned int>
{
    using super = Vector<double,unsigned int>;
    using super::super;

    void close() override { PYBIND11_OVERLOAD_PURE(void, super, close ); }
    void zero() override { PYBIND11_OVERLOAD_PURE( void, super, zero ); }
    void zero(size_type start, size_type stop) override {
        PYBIND11_OVERLOAD_PURE( void, super, zero, start, stop );
    }
    void setConstant(value_type v) override { PYBIND11_OVERLOAD_PURE( void, super, setConstant, v );}
    clone_ptrtype clone() const override { PYBIND11_OVERLOAD_PURE( clone_ptrtype, super, clone ); }
    value_type sum() const override { PYBIND11_OVERLOAD_PURE( value_type, super, sum ); }
    real_type min() const override { PYBIND11_OVERLOAD_PURE( real_type, super, min ); }
    real_type max() const override { PYBIND11_OVERLOAD_PURE( real_type, super, max ); }
    real_type l1Norm() const override { PYBIND11_OVERLOAD_PURE( real_type, super, l1Norm ); }
    real_type l2Norm() const override { PYBIND11_OVERLOAD_PURE( real_type, super, l2Norm ); }
    real_type linftyNorm() const override { PYBIND11_OVERLOAD_PURE( real_type, super, linftyNorm );}
    double operator()(const size_type i) const override {
        PYBIND11_OVERLOAD_PURE( double, super, operator(), i ); }
    double& operator()(const size_type i) override {
        PYBIND11_OVERLOAD_PURE( double&, super, operator(), i ); }
    super& operator+=(const super& V) override {
        PYBIND11_OVERLOAD_PURE( super&, super, operator+=, V ); }
    super& operator-=(const super& V) override {
        PYBIND11_OVERLOAD_PURE( super&, super, operator-=, V ); }
    void set(const size_type i, const value_type& value) override {
        PYBIND11_OVERLOAD_PURE( void, super, set, i, value ); }
    void setVector(int* i, int n, value_type* v) override {
        PYBIND11_OVERLOAD_PURE( void, super, setVector, i, n, v ); }
    void add(const size_type i, const value_type& value) override {
        PYBIND11_OVERLOAD_PURE( void, super, add, i, value ); }
    void addVector(int* i, int n, value_type* v, size_type K, size_type K2) override {
        PYBIND11_OVERLOAD_PURE( void, super, addVector, i, n, v, K, K2 ); }
    void add(const value_type& s) override {
        PYBIND11_OVERLOAD_PURE( void, super, add, s ); }
    void add(const super& V) override {
        PYBIND11_OVERLOAD_PURE( void, super, add, V ); }
    void add(const value_type& a, const super& V) override {
        PYBIND11_OVERLOAD_PURE( void, super, add, a, V ); }
    void addVector(const std::vector<double>& v, const std::vector<size_type>& dof_indices) override {
        PYBIND11_OVERLOAD_PURE( void, super, addVector, v, dof_indices ); }
    void addVector(const super& v, const std::vector<size_type>& dof_indices) override {
        PYBIND11_OVERLOAD_PURE( void, super, addVector, v, dof_indices ); }
    void addVector(const super& V_in, const MatrixSparse<double>& A_in) override {
        PYBIND11_OVERLOAD_PURE( void, super, addVector, V_in, A_in ); }
    value_type dot( super const& v ) const override {
        PYBIND11_OVERLOAD_PURE( value_type, super, dot, v ); }
    void insert(const std::vector<double>& v, const std::vector<size_type>& dof_indices) override {
        PYBIND11_OVERLOAD_PURE( void, super, insert, v, dof_indices ); }
    void insert(const super& V, const std::vector<size_type>& dof_indices) override {
        PYBIND11_OVERLOAD_PURE( void, super, insert, V, dof_indices ); }
    void insert(const ublas::vector<double>& V, const std::vector<size_type>& dof_indices) override {
        PYBIND11_OVERLOAD_PURE( void, super, insert, V, dof_indices ); }
    void scale(const double factor) override {
        PYBIND11_OVERLOAD_PURE( void, super, scale, factor ); }
    void printMatlab(const std::string name, bool renumber) const override {
        PYBIND11_OVERLOAD_PURE( void, super, printMatlab, name, renumber ); }
};

class PyMatrixSparseDouble : MatrixSparse<double>
{
    using super = MatrixSparse<double>;
    using super::super;
    void init ( const size_type m,
                const size_type n,
                const size_type m_l,
                const size_type n_l,
                const size_type nnz=30,
                const size_type noz=10 ) override {
        PYBIND11_OVERLOAD_PURE(void, super, init, m, n, m_l, n_l, nnz, noz); }
    void init ( const size_type m,
                const size_type n,
                const size_type m_l,
                const size_type n_l,
                graph_ptrtype const& graph ) override {
        PYBIND11_OVERLOAD_PURE(void, super, init, m, n, m_l, n_l); }
    size_type nnz() const override {
        PYBIND11_OVERLOAD_PURE(size_type, super, nnz); }
    void clear () override {
        PYBIND11_OVERLOAD_PURE(void, super, clear); }
    void zero () override {
        PYBIND11_OVERLOAD_PURE(void, super, zero); }
    void zero ( size_type start1, size_type size1,
                size_type start2, size_type size2 ) override {
        PYBIND11_OVERLOAD_PURE(void, super, zero, start1, size1, start2, size2); }
    void close () const override {
        PYBIND11_OVERLOAD_PURE(void, super, close); }
    size_type size1 () const override {
        PYBIND11_OVERLOAD_PURE(size_type, super, size1); }
    size_type size2 () const override {
        PYBIND11_OVERLOAD_PURE(size_type, super, size2); }
    size_type rowStart () const override {
        PYBIND11_OVERLOAD_PURE(size_type, super, rowStart); }
    size_type rowStop () const override {
        PYBIND11_OVERLOAD_PURE(size_type, super, rowStop); }
    void set ( const size_type i,
               const size_type j,
               const value_type& value ) override {
        PYBIND11_OVERLOAD_PURE(void, super, set, i, j, value); }
    void add ( const size_type i,
               const size_type j,
               const value_type& value ) override {
        PYBIND11_OVERLOAD_PURE(void, super, add, i, j, value); }
    void addMatrix ( const ublas::matrix<value_type> &dm,
                     const std::vector<size_type> &rows,
                     const std::vector<size_type> &cols ) override {
        PYBIND11_OVERLOAD_PURE(void, super, addMatrix, dm, rows, cols); }
    void addMatrix ( int* rows, int nrows,
                     int* cols, int ncols,
                     value_type* data, size_type K, size_type K2 ) override {
        PYBIND11_OVERLOAD_PURE(void, super, addMatrix, rows, nrows, cols, ncols, data, K, K2); }
    void addMatrix ( const ublas::matrix<value_type> &dm,
                     const std::vector<size_type> &dof_indices ) override {
        PYBIND11_OVERLOAD_PURE(void, super, addMatrix, dm, dof_indices); }
    void addMatrix ( const double f, super const& m, Feel::MatrixStructure matStruc = Feel::SAME_NONZERO_PATTERN ) override {
        PYBIND11_OVERLOAD_PURE(void, super, addMatrix, f, m, matStruc); }
    void scale ( const double f ) override {
        PYBIND11_OVERLOAD_PURE(void, super, scale, f); }
    double operator () ( const size_type i,
                         const size_type j ) const override {
        PYBIND11_OVERLOAD_PURE(double, super, operator (), i, j); }
    super& operator = ( super const& M ) override {
        PYBIND11_OVERLOAD_PURE(super&, super,  operator=); }
    void diagonal ( Vector<double>& dest ) const override {
        PYBIND11_OVERLOAD_PURE(void, super, diagonal, dest); }
    void transpose( MatrixSparse<value_type>& Mt, size_type options = MATRIX_TRANSPOSE_ASSEMBLED ) const override {
        PYBIND11_OVERLOAD_PURE(void, super, transpose, Mt, options); }
    real_type energy ( vector_type const& v,
                       vector_type const& u,
                       bool transpose = false ) const override {
        PYBIND11_OVERLOAD_PURE(real_type, super, energy, u, v, transpose); }
    real_type l1Norm () const override {
        PYBIND11_OVERLOAD_PURE(real_type, super, l1Norm); }
    real_type linftyNorm () const override {
        PYBIND11_OVERLOAD_PURE(real_type, super, linftyNorm); }
    void zeroRows( std::vector<int> const& rows,
                   Vector<value_type> const& values,
                   Vector<value_type>& rhs,
                   Context const& on_context,
                   value_type value_on_diagonal ) override {
        PYBIND11_OVERLOAD_PURE(void, super, zeroRows, rows, values, rhs, on_context, value_on_diagonal); }
    void updateBlockMat( std::shared_ptr<super > const& m, std::vector<size_type> const& start_i, std::vector<size_type> const& start_j ) override {
        PYBIND11_OVERLOAD_PURE(void, super, updateBlockMat, m, start_i, start_j); }
};

PYBIND11_MODULE(_alg, m )
{
    using namespace Feel;

    if (import_mpi4py()<0) return ;

    py::class_<Vector<double, uint32_type>, PyVectorDouble, std::shared_ptr<Vector<double, uint32_type>> >(m, "VectorDouble")
        .def(py::init<>())
        ;
    py::class_<VectorPetsc<double>, Vector<double,uint32_type>, std::shared_ptr<VectorPetsc<double> > >(m, "VectorPetscDouble")
        .def(py::init<>())
        ;
    py::class_<MatrixSparse<double>, PyMatrixSparseDouble, std::shared_ptr<MatrixSparse<double>> >(m, "MatrixSparseDouble")
        .def(py::init<>())
        ;
    py::class_<MatrixPetsc<double>, MatrixSparse<double>, std::shared_ptr<MatrixPetsc<double> > >(m, "MatrixPetscDouble")
        .def(py::init<>())
        ;

}

