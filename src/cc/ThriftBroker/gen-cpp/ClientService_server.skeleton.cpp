// This autogenerated skeleton file illustrates how to build a server.
// You should copy it to another filename to avoid overwriting it.

#include "ClientService.h"
#include <protocol/TBinaryProtocol.h>
#include <server/TSimpleServer.h>
#include <transport/TServerSocket.h>
#include <transport/TBufferTransports.h>

using namespace ::apache::thrift;
using namespace ::apache::thrift::protocol;
using namespace ::apache::thrift::transport;
using namespace ::apache::thrift::server;

using boost::shared_ptr;

using namespace Hypertable::ThriftGen;

class ClientServiceHandler : virtual public ClientServiceIf {
 public:
  ClientServiceHandler() {
    // Your initialization goes here
  }

  void create_namespace(const std::string& ns) {
    // Your implementation goes here
    printf("create_namespace\n");
  }

  void create_table(const Namespace ns, const std::string& table_name, const std::string& schema) {
    // Your implementation goes here
    printf("create_table\n");
  }

  void alter_table(const Namespace ns, const std::string& table_name, const std::string& schema) {
    // Your implementation goes here
    printf("alter_table\n");
  }

  Namespace open_namespace(const std::string& ns) {
    // Your implementation goes here
    printf("open_namespace\n");
  }

  void close_namespace(const Namespace ns) {
    // Your implementation goes here
    printf("close_namespace\n");
  }

  Future open_future(const int32_t queue_size) {
    // Your implementation goes here
    printf("open_future\n");
  }

  void cancel_future(const Future ff) {
    // Your implementation goes here
    printf("cancel_future\n");
  }

  void get_future_result(Result& _return, const Future ff) {
    // Your implementation goes here
    printf("get_future_result\n");
  }

  void get_future_result_as_arrays(ResultAsArrays& _return, const Future ff) {
    // Your implementation goes here
    printf("get_future_result_as_arrays\n");
  }

  void get_future_result_serialized(ResultSerialized& _return, const Future ff) {
    // Your implementation goes here
    printf("get_future_result_serialized\n");
  }

  bool future_is_empty(const Future ff) {
    // Your implementation goes here
    printf("future_is_empty\n");
  }

  bool future_is_full(const Future ff) {
    // Your implementation goes here
    printf("future_is_full\n");
  }

  bool future_is_cancelled(const Future ff) {
    // Your implementation goes here
    printf("future_is_cancelled\n");
  }

  bool future_has_outstanding(const Future ff) {
    // Your implementation goes here
    printf("future_has_outstanding\n");
  }

  void close_future(const Future ff) {
    // Your implementation goes here
    printf("close_future\n");
  }

  Scanner open_scanner(const Namespace ns, const std::string& table_name, const ScanSpec& scan_spec) {
    // Your implementation goes here
    printf("open_scanner\n");
  }

  ScannerAsync open_scanner_async(const Namespace ns, const std::string& table_name, const Future future, const ScanSpec& scan_spec) {
    // Your implementation goes here
    printf("open_scanner_async\n");
  }

  void close_scanner(const Scanner scanner) {
    // Your implementation goes here
    printf("close_scanner\n");
  }

  void close_scanner_async(const ScannerAsync scanner) {
    // Your implementation goes here
    printf("close_scanner_async\n");
  }

  void next_cells(std::vector<Cell> & _return, const Scanner scanner) {
    // Your implementation goes here
    printf("next_cells\n");
  }

  void next_cells_as_arrays(std::vector<CellAsArray> & _return, const Scanner scanner) {
    // Your implementation goes here
    printf("next_cells_as_arrays\n");
  }

  void next_cells_serialized(CellsSerialized& _return, const Scanner scanner) {
    // Your implementation goes here
    printf("next_cells_serialized\n");
  }

  void next_row(std::vector<Cell> & _return, const Scanner scanner) {
    // Your implementation goes here
    printf("next_row\n");
  }

  void next_row_as_arrays(std::vector<CellAsArray> & _return, const Scanner scanner) {
    // Your implementation goes here
    printf("next_row_as_arrays\n");
  }

  void next_row_serialized(CellsSerialized& _return, const Scanner scanner) {
    // Your implementation goes here
    printf("next_row_serialized\n");
  }

  void get_row(std::vector<Cell> & _return, const Namespace ns, const std::string& table_name, const std::string& row) {
    // Your implementation goes here
    printf("get_row\n");
  }

  void get_row_as_arrays(std::vector<CellAsArray> & _return, const Namespace ns, const std::string& name, const std::string& row) {
    // Your implementation goes here
    printf("get_row_as_arrays\n");
  }

  void get_row_serialized(CellsSerialized& _return, const Namespace ns, const std::string& table_name, const std::string& row) {
    // Your implementation goes here
    printf("get_row_serialized\n");
  }

  void get_cell(Value& _return, const Namespace ns, const std::string& table_name, const std::string& row, const std::string& column) {
    // Your implementation goes here
    printf("get_cell\n");
  }

  void get_cells(std::vector<Cell> & _return, const Namespace ns, const std::string& table_name, const ScanSpec& scan_spec) {
    // Your implementation goes here
    printf("get_cells\n");
  }

  void get_cells_as_arrays(std::vector<CellAsArray> & _return, const Namespace ns, const std::string& name, const ScanSpec& scan_spec) {
    // Your implementation goes here
    printf("get_cells_as_arrays\n");
  }

  void get_cells_serialized(CellsSerialized& _return, const Namespace ns, const std::string& name, const ScanSpec& scan_spec) {
    // Your implementation goes here
    printf("get_cells_serialized\n");
  }

  void refresh_shared_mutator(const Namespace ns, const std::string& table_name, const MutateSpec& mutate_spec) {
    // Your implementation goes here
    printf("refresh_shared_mutator\n");
  }

  void offer_cells(const Namespace ns, const std::string& table_name, const MutateSpec& mutate_spec, const std::vector<Cell> & cells) {
    // Your implementation goes here
    printf("offer_cells\n");
  }

  void offer_cells_as_arrays(const Namespace ns, const std::string& table_name, const MutateSpec& mutate_spec, const std::vector<CellAsArray> & cells) {
    // Your implementation goes here
    printf("offer_cells_as_arrays\n");
  }

  void offer_cell(const Namespace ns, const std::string& table_name, const MutateSpec& mutate_spec, const Cell& cell) {
    // Your implementation goes here
    printf("offer_cell\n");
  }

  void offer_cell_as_array(const Namespace ns, const std::string& table_name, const MutateSpec& mutate_spec, const CellAsArray& cell) {
    // Your implementation goes here
    printf("offer_cell_as_array\n");
  }

  Mutator open_mutator(const Namespace ns, const std::string& table_name, const int32_t flags, const int32_t flush_interval) {
    // Your implementation goes here
    printf("open_mutator\n");
  }

  MutatorAsync open_mutator_async(const Namespace ns, const std::string& table_name, const Future future, const int32_t flags) {
    // Your implementation goes here
    printf("open_mutator_async\n");
  }

  void close_mutator(const Mutator mutator) {
    // Your implementation goes here
    printf("close_mutator\n");
  }

  void close_mutator_async(const MutatorAsync mutator) {
    // Your implementation goes here
    printf("close_mutator_async\n");
  }

  void set_cell(const Mutator mutator, const Cell& cell) {
    // Your implementation goes here
    printf("set_cell\n");
  }

  void set_cell_as_array(const Mutator mutator, const CellAsArray& cell) {
    // Your implementation goes here
    printf("set_cell_as_array\n");
  }

  void set_cells(const Mutator mutator, const std::vector<Cell> & cells) {
    // Your implementation goes here
    printf("set_cells\n");
  }

  void set_cells_as_arrays(const Mutator mutator, const std::vector<CellAsArray> & cells) {
    // Your implementation goes here
    printf("set_cells_as_arrays\n");
  }

  void set_cells_serialized(const Mutator mutator, const CellsSerialized& cells, const bool flush) {
    // Your implementation goes here
    printf("set_cells_serialized\n");
  }

  void flush_mutator(const Mutator mutator) {
    // Your implementation goes here
    printf("flush_mutator\n");
  }

  void set_cell_async(const MutatorAsync mutator, const Cell& cell) {
    // Your implementation goes here
    printf("set_cell_async\n");
  }

  void set_cell_as_array_async(const MutatorAsync mutator, const CellAsArray& cell) {
    // Your implementation goes here
    printf("set_cell_as_array_async\n");
  }

  void set_cells_async(const MutatorAsync mutator, const std::vector<Cell> & cells) {
    // Your implementation goes here
    printf("set_cells_async\n");
  }

  void set_cells_as_arrays_async(const MutatorAsync mutator, const std::vector<CellAsArray> & cells) {
    // Your implementation goes here
    printf("set_cells_as_arrays_async\n");
  }

  void set_cells_serialized_async(const MutatorAsync mutator, const CellsSerialized& cells, const bool flush) {
    // Your implementation goes here
    printf("set_cells_serialized_async\n");
  }

  void flush_mutator_async(const MutatorAsync mutator) {
    // Your implementation goes here
    printf("flush_mutator_async\n");
  }

  bool exists_namespace(const std::string& ns) {
    // Your implementation goes here
    printf("exists_namespace\n");
  }

  bool exists_table(const Namespace ns, const std::string& name) {
    // Your implementation goes here
    printf("exists_table\n");
  }

  void get_table_id(std::string& _return, const Namespace ns, const std::string& table_name) {
    // Your implementation goes here
    printf("get_table_id\n");
  }

  void get_schema_str(std::string& _return, const Namespace ns, const std::string& table_name) {
    // Your implementation goes here
    printf("get_schema_str\n");
  }

  void get_schema_str_with_ids(std::string& _return, const Namespace ns, const std::string& table_name) {
    // Your implementation goes here
    printf("get_schema_str_with_ids\n");
  }

  void get_schema(Schema& _return, const Namespace ns, const std::string& table_name) {
    // Your implementation goes here
    printf("get_schema\n");
  }

  void get_tables(std::vector<std::string> & _return, const Namespace ns) {
    // Your implementation goes here
    printf("get_tables\n");
  }

  void get_listing(std::vector<NamespaceListing> & _return, const Namespace ns) {
    // Your implementation goes here
    printf("get_listing\n");
  }

  void get_table_splits(std::vector<TableSplit> & _return, const Namespace ns, const std::string& table_name) {
    // Your implementation goes here
    printf("get_table_splits\n");
  }

  void drop_namespace(const std::string& ns, const bool if_exists) {
    // Your implementation goes here
    printf("drop_namespace\n");
  }

  void rename_table(const Namespace ns, const std::string& name, const std::string& new_name) {
    // Your implementation goes here
    printf("rename_table\n");
  }

  void drop_table(const Namespace ns, const std::string& name, const bool if_exists) {
    // Your implementation goes here
    printf("drop_table\n");
  }

};

int main(int argc, char **argv) {
  int port = 9090;
  shared_ptr<ClientServiceHandler> handler(new ClientServiceHandler());
  shared_ptr<TProcessor> processor(new ClientServiceProcessor(handler));
  shared_ptr<TServerTransport> serverTransport(new TServerSocket(port));
  shared_ptr<TTransportFactory> transportFactory(new TBufferedTransportFactory());
  shared_ptr<TProtocolFactory> protocolFactory(new TBinaryProtocolFactory());

  TSimpleServer server(processor, serverTransport, transportFactory, protocolFactory);
  server.serve();
  return 0;
}

