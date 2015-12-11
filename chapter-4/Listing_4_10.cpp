// [Listing 4.10]
// 단일 스레드에서 promise를 사용해 다수의 연결 다루기

#include <future>
void process_connections(connection_set& connections)
{
     // 1. 
     while(!done(connections))
     {
          // 2. 
          for(connection_iterator
               connection=connections.begin(),end=connections.end();
               connection!=end;
               ++connection)
          {
               // 3. 
               if(connection->has_incoming_data())
               {
                    data_packet data=connection->incoming();
                    std::promise<payload_type>& p=
                         // 4. 
                         connection->get_promise(data.id);
                    p.set_value(data.payload);
               }
               // 5. 
               if(connection->has_outgoing_data())
               {
                    outgoing_packet data=
                         connection->top_of_outgoing_queue();
                    connection->send(data.payload);
                    // 6.
                    data.promise.set_value(true);
               }
          }
     }
}
