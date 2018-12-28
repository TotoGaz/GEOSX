/*
 *~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * Copyright (c) 2018, Lawrence Livermore National Security, LLC.
 *
 * Produced at the Lawrence Livermore National Laboratory
 *
 * LLNL-CODE-746361
 *
 * All rights reserved. See COPYRIGHT for details.
 *
 * This file is part of the GEOSX Simulation Framework.
 *
 * GEOSX is a free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License (as published by the
 * Free Software Foundation) version 2.1 dated February 1999.
 *~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 */

/**
  * @file BrooksCoreyRelativePermeability.cpp
  */

#include "BrooksCoreyRelativePermeability.hpp"

#include <cmath>

namespace geosx
{

using namespace dataRepository;
using namespace cxx_utilities;

namespace constitutive
{


BrooksCoreyRelativePermeability::BrooksCoreyRelativePermeability( std::string const & name,
                                                                  ManagedGroup * const parent )
  : RelativePermeabilityBase( name, parent )
{
  RegisterViewWrapper( viewKeyStruct::phaseMinVolumeFractionString, &m_phaseMinVolumeFraction, false )->
      setDefaultValue(0.0)->setToDefaultValue()->
      setInputFlag(InputFlags::OPTIONAL)->
      setDescription("Minimum volume fraction value for each phase");

  RegisterViewWrapper( viewKeyStruct::phaseRelPermExponentString,   &m_phaseRelPermExponent,   false )->
      setDefaultValue(1.0)->setToDefaultValue()->
      setInputFlag(InputFlags::OPTIONAL)->
      setDescription("MinimumRel perm power law exponent for each phase");


  RegisterViewWrapper( viewKeyStruct::phaseRelPermMaxValueString,   &m_phaseRelPermMaxValue,   false )->
      setDefaultValue(0.0)->setToDefaultValue()->
      setInputFlag(InputFlags::OPTIONAL)->
      setDescription("Maximum rel perm value for each phase");

}

BrooksCoreyRelativePermeability::~BrooksCoreyRelativePermeability()
{

}

std::unique_ptr<ConstitutiveBase>
BrooksCoreyRelativePermeability::DeliverClone(string const & name, ManagedGroup * const parent) const
{
  std::unique_ptr< BrooksCoreyRelativePermeability > clone = std::make_unique<BrooksCoreyRelativePermeability>( name, parent );

  clone->m_phaseNames = this->m_phaseNames;
  clone->m_phaseTypes = this->m_phaseTypes;
  clone->m_phaseOrder = this->m_phaseOrder;

  clone->m_phaseMinVolumeFraction = this->m_phaseMinVolumeFraction;
  clone->m_phaseRelPermExponent   = this->m_phaseRelPermExponent;
  clone->m_phaseRelPermMaxValue   = this->m_phaseRelPermMaxValue;

  clone->m_satScale = this->m_satScale;

  std::unique_ptr<ConstitutiveBase> rval = std::move( clone );
  return rval;
}


void BrooksCoreyRelativePermeability::ProcessInputFile_PostProcess()
{
  RelativePermeabilityBase::ProcessInputFile_PostProcess();

  localIndex const NP = numFluidPhases();

#define COREY_CHECK_INPUT_LENGTH( data, expected, attr ) \
  if (integer_conversion<localIndex>((data).size()) != integer_conversion<localIndex>(expected)) \
  { \
    GEOS_ERROR( "BrooksCoreyRelativePermeability: invalid number of entries in " \
                << (attr) << " attribute (" \
                << (data).size() << "given, " \
                << (expected) << " expected)"); \
  }

  COREY_CHECK_INPUT_LENGTH( m_phaseMinVolumeFraction, NP, viewKeyStruct::phaseMinVolumeFractionString );
  COREY_CHECK_INPUT_LENGTH( m_phaseRelPermExponent, NP, viewKeyStruct::phaseRelPermExponentString );
  COREY_CHECK_INPUT_LENGTH( m_phaseRelPermMaxValue, NP, viewKeyStruct::phaseRelPermMaxValueString );

#undef COREY_CHECK_INPUT_LENGTH

  m_satScale = 1.0;
  for (localIndex ip = 0; ip < NP; ++ip)
  {
    GEOS_ERROR_IF( m_phaseMinVolumeFraction[ip] < 0.0 || m_phaseMinVolumeFraction[ip] > 1.0,
                   "BrooksCoreyRelativePermeability: invalid min volume fraction value: " << m_phaseMinVolumeFraction[ip] );
    m_satScale -= m_phaseMinVolumeFraction[ip];

    GEOS_ERROR_IF( m_phaseRelPermExponent[ip] < 0.0,
                   "BrooksCoreyRelativePermeability: invalid exponent value: " << m_phaseRelPermExponent[ip] );

    GEOS_ERROR_IF( m_phaseRelPermMaxValue[ip] < 0.0 || m_phaseRelPermMaxValue[ip] > 1.0,
                   "BrooksCoreyRelativePermeability: invalid maximum value: " << m_phaseRelPermMaxValue[ip] );
  }

  GEOS_ERROR_IF( m_satScale < 0.0, "BrooksCoreyRelativePermeability: sum of min volume fractions exceeds 1.0" );
}

void BrooksCoreyRelativePermeability::StateUpdatePointRelPerm( arraySlice1d<real64 const> const & phaseVolFraction,
                                                               localIndex const k,
                                                               localIndex const q )
{
  arraySlice1d<real64> relPerm           = m_phaseRelPerm[k][q];
  arraySlice2d<real64> dRelPerm_dVolFrac = m_dPhaseRelPerm_dPhaseVolFrac[k][q];

  localIndex const NP = numFluidPhases();

  for (localIndex ip = 0; ip < NP; ++ip)
  {
    for (localIndex jp = 0; jp < NP; ++jp)
    {
      dRelPerm_dVolFrac[ip][jp] = 0.0;
    }
  }
  real64 const satScaleInv = 1.0 / m_satScale;

  for (localIndex ip = 0; ip < NP; ++ip)
  {
    real64 const satScaled = (phaseVolFraction[ip] - m_phaseMinVolumeFraction[ip]) * satScaleInv;
    real64 const exponent  = m_phaseRelPermExponent[ip];
    real64 const scale     = m_phaseRelPermMaxValue[ip];

    if (satScaled > 0.0 && satScaled < 1.0)
    {
      // intermediate value
      real64 const v = scale * std::pow( satScaled, exponent - 1.0 );

      relPerm[ip] = v * satScaled;
      dRelPerm_dVolFrac[ip][ip] = v * exponent * satScaleInv;
    }
    else
    {
      relPerm[ip] = (satScaled < 0.0) ? 0.0 : scale;
    }
  }
}

REGISTER_CATALOG_ENTRY( ConstitutiveBase, BrooksCoreyRelativePermeability, std::string const &, ManagedGroup * const )
} // namespace constitutive

} // namespace geosx
