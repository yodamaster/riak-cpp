#include <riak/client.hxx>
#include <riak/request_with_timeout.hxx>

//=============================================================================
namespace riak {
//=============================================================================

// The default does not specify anything out of the ordinary with the configuration of the
// Riak servers themselves. The default is actually server-side.
const object_access_parameters client::access_override_defaults = object_access_parameters();


const request_failure_parameters client::failure_defaults = request_failure_parameters()
    .with_response_timeout(std::chrono::milliseconds(3000))
    .with_retries_permitted(1);


std::shared_ptr<client> make_client (
        transport& t,
        boost::asio::io_service& ios,
        const request_failure_parameters& failure_defaults,
        const object_access_parameters& access_override_defaults)
{
    std::shared_ptr<client> ptr(new client(t, ios, failure_defaults, access_override_defaults));
    return ptr;
}


client::client (
        transport& t,
        boost::asio::io_service& ios,
        const request_failure_parameters& fp,
        const object_access_parameters& d)
  : transport_(t),
    access_overrides_(d),
    request_failure_defaults_(fp),
    ios_(ios)
{   }


bucket client::bucket (const key& k)
{
    assert(this);
    return bucket::bucket(shared_from_this(), k, request_failure_defaults_, access_overrides_);
}


void client::transmit_request(const std::string& body, message::buffering_handler& h, std::chrono::milliseconds timeout)
{
    assert(this);
    std::shared_ptr<request_with_timeout> request(new request_with_timeout(body, timeout, h, ios_));
    request->dispatch_via(transport_);
}

//=============================================================================
}   // namespace riak
//=============================================================================