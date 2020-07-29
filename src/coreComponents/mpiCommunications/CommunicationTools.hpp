/*
 * ------------------------------------------------------------------------------------------------------------
 * SPDX-License-Identifier: LGPL-2.1-only
 *
 * Copyright (c) 2018-2020 Lawrence Livermore National Security LLC
 * Copyright (c) 2018-2020 The Board of Trustees of the Leland Stanford Junior University
 * Copyright (c) 2018-2020 Total, S.A
 * Copyright (c) 2019-     GEOSX Contributors
 * All rights reserved
 *
 * See top level LICENSE, COPYRIGHT, CONTRIBUTORS, NOTICE, and ACKNOWLEDGEMENTS files for details.
 * ------------------------------------------------------------------------------------------------------------
 */

/**
 * @file CommunicationTools.hpp
 */

#ifndef GEOSX_MPICOMMUNICATIONS_COMMUNICATIONTOOLS_HPP_
#define GEOSX_MPICOMMUNICATIONS_COMMUNICATIONTOOLS_HPP_

#include "MpiWrapper.hpp"

#include "common/DataTypes.hpp"
#include "managers/GeosxState.hpp"

#include <set>

namespace geosx
{


class ObjectManagerBase;
class NeighborCommunicator;
class MeshLevel;
class ElementRegionManager;

class MPI_iCommData;

class CommID
{
public:
  CommID( std::set< int > & freeIDs ):
    m_freeIDs( freeIDs ),
    m_id(-1)
  {
    GEOSX_ERROR_IF_GE( freeIDs.size(), 0 );
    m_id = *freeIDs.begin();
    freeIDs.erase( freeIDs.begin() );
  }

  CommID( CommID && src ):
    m_freeIDs( src.m_freeIDs ),
    m_id( src.m_id )
  {
    src.m_id = -1;
  }

  ~CommID()
  {
    if ( m_id < 0 )
    { return; }

    GEOSX_ERROR_IF( m_freeIDs.count( m_id ) > 0, "Attempting to release commID that is already free: " << m_id );

    m_freeIDs.insert( m_id );
    m_id = -1;
  }

  CommID( CommID const & ) = delete;
  CommID & operator=( CommID const & ) = delete;
  CommID & operator=( CommID && ) = delete;

  constexpr operator int()
  { return m_id; }

private:
  std::set< int > & m_freeIDs;
  int m_id = -1;
};


class CommunicationTools
{
public:
  CommunicationTools();
  ~CommunicationTools();

  void AssignGlobalIndices( ObjectManagerBase & object,
                                   ObjectManagerBase const & compositionObject,
                                   std::vector< NeighborCommunicator > & neighbors );

  void AssignNewGlobalIndices( ObjectManagerBase & object,
                                      std::set< localIndex > const & indexList );

  void
  AssignNewGlobalIndices( ElementRegionManager & elementManager,
                          std::map< std::pair< localIndex, localIndex >, std::set< localIndex > > const & newElems );

  void FindGhosts( MeshLevel & meshLevel,
                          std::vector< NeighborCommunicator > & neighbors,
                          bool use_nonblocking );

  CommID getCommID()
  { return CommID( m_freeCommIDs ); }

  void FindMatchedPartitionBoundaryObjects( ObjectManagerBase * const group,
                                                   std::vector< NeighborCommunicator > & allNeighbors );

  void SynchronizeFields( const std::map< string, string_array > & fieldNames,
                                 MeshLevel * const mesh,
                                 std::vector< NeighborCommunicator > & allNeighbors,
                                 bool on_device = false );

  void SynchronizePackSendRecvSizes( const std::map< string, string_array > & fieldNames,
                                            MeshLevel * const mesh,
                                            std::vector< NeighborCommunicator > & neighbors,
                                            MPI_iCommData & icomm,
                                            bool on_device = false );

  void SynchronizePackSendRecv( const std::map< string, string_array > & fieldNames,
                                       MeshLevel * const mesh,
                                       std::vector< NeighborCommunicator > & allNeighbors,
                                       MPI_iCommData & icomm,
                                       bool on_device = false );

  void SynchronizeUnpack( MeshLevel * const mesh,
                                 std::vector< NeighborCommunicator > & neighbors,
                                 MPI_iCommData & icomm,
                                 bool on_device = false );

private:
  std::set< int > m_freeCommIDs;
};


class MPI_iCommData
{
public:

  MPI_iCommData():
    size( 0 ),
    commID( getGlobalState().getCommunicationTools().getCommID() ),
    sizeCommID( getGlobalState().getCommunicationTools().getCommID() ),
    fieldNames(),
    mpiSendBufferRequest(),
    mpiRecvBufferRequest(),
    mpiSendBufferStatus(),
    mpiRecvBufferStatus()
  {}

  ~MPI_iCommData()
  {}

  void resize( localIndex numMessages )
  {
    mpiSendBufferRequest.resize( numMessages );
    mpiRecvBufferRequest.resize( numMessages );
    mpiSendBufferStatus.resize( numMessages );
    mpiRecvBufferStatus.resize( numMessages );
    mpiSizeSendBufferRequest.resize( numMessages );
    mpiSizeRecvBufferRequest.resize( numMessages );
    mpiSizeSendBufferStatus.resize( numMessages );
    mpiSizeRecvBufferStatus.resize( numMessages );
    size = static_cast< int >(numMessages);
  }

  int size;
  int commID;
  int sizeCommID;
  std::map< string, string_array > fieldNames;

  array1d< MPI_Request > mpiSendBufferRequest;
  array1d< MPI_Request > mpiRecvBufferRequest;
  array1d< MPI_Status >  mpiSendBufferStatus;
  array1d< MPI_Status >  mpiRecvBufferStatus;

  array1d< MPI_Request > mpiSizeSendBufferRequest;
  array1d< MPI_Request > mpiSizeRecvBufferRequest;
  array1d< MPI_Status >  mpiSizeSendBufferStatus;
  array1d< MPI_Status >  mpiSizeRecvBufferStatus;
};



} /* namespace geosx */

#endif /* GEOSX_MPICOMMUNICATIONS_COMMUNICATIONTOOLS_HPP_ */
