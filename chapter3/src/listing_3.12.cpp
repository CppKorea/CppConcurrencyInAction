#include <mutex>
#include <iostream>

/* 실행안됨 고치고 다시 올리겠음 */

struct connection_info
{
	int port;
};

struct data_packet
{
	int op;
};

struct connection_handle
{
    void send_data(data_packet const&)
    {}
    data_packet receive_data()
    {
        return data_packet();
    }
};

struct remote_connection_manager
{
    connection_handle open(connection_info const&)
    {
        return connection_handle();
    }
} connection_manager;


class X
{
private:
    connection_info connection_details;
    connection_handle connection;
    std::once_flag connection_init_flag;

    void open_connection()
    {
        connection=connection_manager.open(connection_details);
		std::cout << "initialize" << std::endl;
    }
public:
    X(connection_info const& connection_details_):
        connection_details(connection_details_) {}
	~X() {}

    void send_data(data_packet const& data)
    {
		/* c++11 에서는 std::call_once 대신 static 로컬 변수를 사용해도 thread safe 하게 한번만 초기화 된다. */
        std::call_once( connection_init_flag, &X::open_connection, this );
    	connection.send_data( data );
	}
    data_packet receive_data()
    {
        std::call_once( connection_init_flag, &X::open_connection, this );
		return connection.receive_data();
	}
};

int main()
{
	connection_info ci;
	data_packet dp;
	data_packet rdp;
	X a(ci);

	a.send_data(dp);
	//rdp = a.receive_data();

	return 0;
}
