/*
 * ------------------------------------------------------------------------------------------------------------
 * SPDX-License-Identifier: LGPL-2.1-only
 *
 * Copyright (c) 2018-2019 Lawrence Livermore National Security LLC
 * Copyright (c) 2018-2019 The Board of Trustees of the Leland Stanford Junior University
 * Copyright (c) 2018-2019 Total, S.A
 * Copyright (c) 2019-     GEOSX Contributors
 * All right reserved
 *
 * See top level LICENSE, COPYRIGHT, CONTRIBUTORS, NOTICE, and ACKNOWLEDGEMENTS files for details.
 * ------------------------------------------------------------------------------------------------------------
 */

/**
 * @file HyprePreconditioner.hpp
 */

#ifndef GEOSX_HYPREPRECONDITIONER_HPP
#define GEOSX_HYPREPRECONDITIONER_HPP

#include "linearAlgebra/solvers/PreconditionerBase.hpp"
#include "linearAlgebra/interfaces/hypre/HypreInterface.hpp"
#include "linearAlgebra/utilities/LinearSolverParameters.hpp"
#include "HypreUtils.hpp"

#include <memory>

/**
 * @name Hypre forward declarations.
 *
 * Forward declare hypre's solver structs and pointer aliases in order
 * to avoid including hypre headers and leaking into the rest of GEOSX.
 */
///@{

/// Hypre solver struct forward declaration
extern "C" struct hypre_Solver_struct;

/// Solver pointer alias
using HYPRE_Solver = hypre_Solver_struct *;

///@}

namespace geosx
{

/// Forward-declared struct that hosts preconditioner auxiliary data
struct HyprePrecAuxData;

/// Forward-declared struct that hosts pointers to preconditioner functions
struct HyprePrecFuncs;

/**
 * @brief Wrapper around hypre-based preconditioners.
 */
class HyprePreconditioner final : public PreconditionerBase< HypreInterface >
{
public:

  /// Alias for base type
  using Base = PreconditionerBase< HypreInterface >;

  /// Alias for vector type
  using Vector = typename Base::Vector;

  /// Alias for matrix type
  using Matrix = typename Base::Matrix;

  /// Allow for partial overload of Base::compute()
  using Base::compute;

  /**
   * @brief Constructor.
   * @param params preconditioner parameters
   * @param dofManager the Degree-of-Freedom manager associated with matrix
   */
  explicit HyprePreconditioner( LinearSolverParameters params,
                                DofManager const * const dofManager = nullptr );

  /**
   * @brief Constructor.
   * @param params preconditioner parameters
   * @param rigidBodyModes the elasticity near null kernel
   * @param dofManager the Degree-of-Freedom manager associated with matrix
   */
  HyprePreconditioner( LinearSolverParameters params,
                       array1d< HypreVector > const & rigidBodyModes,
                       DofManager const * const dofManager = nullptr );

  /**
   * @brief Destructor.
   */
  virtual ~HyprePreconditioner() override;

  /**
   * @brief Compute the preconditioner from a matrix.
   * @param mat the matrix to precondition.
   */
  virtual void compute( Matrix const & mat ) override;

  /**
   * @brief Apply operator to a vector
   * @param src Input vector (x).
   * @param dst Output vector (b).
   *
   * @warning @p src and @p dst cannot alias the same vector.
   */
  virtual void apply( Vector const & src, Vector & dst ) const override;

  virtual void clear() override;

  /**
   * @brief Access the underlying implementation.
   * @return reference to hypre preconditioner
   */
  HYPRE_Solver const & unwrapped() const;

  /**
   * @brief Access the underlying implementation.
   * @return reference to container of preconditioner functions.
   *
   * Intended for use by HypreSolver.
   */
  HyprePrecFuncs const & unwrappedFuncs() const;

private:

  void createHyprePreconditioner( DofManager const * const dofManager );

  void createAMG();

  void createMGR( DofManager const * const dofManager );

  void createILU();

  void createILUT();

  /// Parameters for all preconditioners
  LinearSolverParameters m_parameters;

  /// Pointer to the Hypre implementation
  HYPRE_Solver m_precond = nullptr;

  /// Pointer to the auxillary preconditioner used in MGR
  HYPRE_Solver aux_precond = nullptr;

  /// Pointers to hypre functions to setup/solve/destroy preconditioner
  std::unique_ptr< HyprePrecFuncs > m_functions;

  /// Pointer to preconditioner auxiliary data
  std::unique_ptr< HyprePrecAuxData > m_auxData;

  /// Pointer to Degrees of Freedom manager
  DofManager const * const m_dofManager;

  /// Bool to check if the preconditioner is ready to be computed
  bool m_ready;

  /// Pointer to external data structure storing rigid body modes
  array1d< HypreVector > const * m_rigidBodyModes = nullptr;

  /// Number of rigid body modes (rotations only)
  HYPRE_Int m_numRotations;

  /// Hypre pointer to the rigid body modes
  array1d< HYPRE_ParVector > m_nullSpacePointer;
};

}

#endif //GEOSX_HYPREPRECONDITIONER_HPP
