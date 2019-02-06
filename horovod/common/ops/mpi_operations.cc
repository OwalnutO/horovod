//
// Created by Travis Addair on 2018-12-14.
//

#include "mpi_operations.h"

namespace horovod {
namespace common {

void MPIContext::Allreduce(const void* buffer_data, int64_t num_elements,
                           TensorTableEntry& first_entry, const void* sendbuff,
                           Communicator comm) {
  int op = MPI_Allreduce(sendbuff != nullptr ? sendbuff : MPI_IN_PLACE, (void*) buffer_data,
                         (int) num_elements,
                         GetMPIDataType(first_entry.tensor),
                         first_entry.tensor->dtype() == HOROVOD_FLOAT16 ? mpi_float16_sum : MPI_SUM,
                         GetMPICommunicator(comm));
  if (op != MPI_SUCCESS) {
    throw std::logic_error("MPI_Allreduce failed, see MPI output for details.");
  }
}

void MPIContext::Allgatherv(const void *sendbuf, int sendcount, DataType sendtype,
                            void *recvbuf, const int recvcounts[],
                            const int displs[], DataType recvtype,
                            Communicator comm) {
  int op = MPI_Allgatherv(sendbuf != nullptr ? sendbuf : MPI_IN_PLACE, sendcount, GetMPIDataType(sendtype),
                          recvbuf, recvcounts, displs, GetMPIDataType(recvtype),
                          GetMPICommunicator(comm));
  if (op != MPI_SUCCESS) {
    throw std::logic_error("MPI_Allgatherv failed, see MPI output for details.");
  }
}

void MPIContext::Broadcast(const void* buffer_data, int64_t num_elements,
                           DataType dtype, int root_rank,
                           Communicator comm) {
  int op = MPI_Bcast((void*) buffer_data,
                     (int) num_elements,
                     GetMPIDataType(dtype),
                     root_rank,
                     GetMPICommunicator(comm));
  if (op != MPI_SUCCESS) {
    throw std::logic_error("MPI_Broadcast failed, see MPI output for details.");
  }
}

void MPIContext::Barrier(Communicator comm) {
  int op = MPI_Barrier(GetMPICommunicator(comm));
  if (op != MPI_SUCCESS) {
    throw std::logic_error("MPI_Barrier failed, see MPI output for details.");
  }
}

void MPIContext::AllocateSharedBuffer(int64_t window_size, int element_size, void* baseptr, Communicator comm) {
  MPI_Win_allocate_shared(
      window_size, element_size, MPI_INFO_NULL, GetMPICommunicator(comm),
      baseptr, &window);
}

void MPIContext::FreeSharedBuffer() {
  MPI_Win_fence(0, window);
  MPI_Win_free(&window);
}

void MPIContext::QuerySharedBuffer(int rank, void* baseptr) {
  int disp_unit;
  MPI_Aint winsize;
  MPI_Win_shared_query(window, rank, &winsize, &disp_unit, baseptr);
}

void MPIContext::GetTypeSize(DataType dtype, int* out) {
  MPI_Type_size(GetMPIDataType(dtype), out);
}

MPI_Datatype MPIContext::GetMPIDataType(const std::shared_ptr<Tensor> tensor) {
  return GetMPIDataType(tensor->dtype());
}

MPI_Datatype MPIContext::GetMPIDataType(const DataType dtype) {
  switch (dtype) {
    case HOROVOD_UINT8:
      return MPI_UINT8_T;
    case HOROVOD_INT8:
      return MPI_INT8_T;
    case HOROVOD_UINT16:
      return MPI_UINT16_T;
    case HOROVOD_INT16:
      return MPI_INT16_T;
    case HOROVOD_INT32:
      return MPI_INT32_T;
    case HOROVOD_INT64:
      return MPI_INT64_T;
    case HOROVOD_FLOAT16:
      return mpi_float16_t;
    case HOROVOD_FLOAT32:
      return MPI_FLOAT;
    case HOROVOD_FLOAT64:
      return MPI_DOUBLE;
    case HOROVOD_BOOL:
      return MPI_C_BOOL;
    case HOROVOD_BYTE:
      return MPI_BYTE;
    case HOROVOD_NULL:
      return MPI_DATATYPE_NULL;
    default:
      throw std::logic_error("Type " + DataType_Name(dtype) +
                             " is not supported in MPI mode.");
  }
}

MPI_Comm MPIContext::GetMPICommunicator(Communicator comm) {
  switch (comm) {
    case GLOBAL:
      return mpi_comm;
    case LOCAL:
      return local_comm;
    case CROSS:
      return cross_comm;
    default:
      throw std::logic_error("Communicator " + CommunicatorName(comm) +
                             " is not supported in MPI mode.");
  }
}

} // namespace common
} // namespace horovod