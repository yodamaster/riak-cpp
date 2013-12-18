#include <boost/log/expressions.hpp>
#include <boost/log/expressions/formatters/date_time.hpp>
#include <boost/log/expressions/formatters/if.hpp>
#include <boost/log/expressions/keyword.hpp>
#include <boost/log/support/date_time.hpp>
#include <boost/log/utility/formatting_ostream.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>
#include <boost/log/utility/setup/file.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <gtest/gtest.h>
#include <riak/log.hxx>

//=============================================================================
namespace riak {
    namespace log {
//=============================================================================

using boost::log::basic_formatting_ostream;

template <typename ch, typename traits>
basic_formatting_ostream<ch, traits>& operator<< (basic_formatting_ostream<ch, traits>& output, const riak::log::severity& level)
{
    output << std::setw(8);

    switch (level) {
        case riak::log::severity::error:   output << "ERROR";   break;
        case riak::log::severity::warning: output << "WARNING"; break;
        case riak::log::severity::info:    output << "INFO";    break;
        case riak::log::severity::trace:   output << "TRACE";   break;
        default:                           output << "???";     break;
    }

    return output;
}

//=============================================================================
    }   // namespace log
}   // namespace riak

namespace {
//=============================================================================

namespace log = boost::log;
namespace expr = boost::log::expressions;

BOOST_LOG_ATTRIBUTE_KEYWORD(severity, "Severity", riak::log::severity);
BOOST_LOG_ATTRIBUTE_KEYWORD(channel, "Channel", riak::log::channel);
BOOST_LOG_ATTRIBUTE_KEYWORD(request_id, "Riak/ClientRequestId", riak::log::request_id_type);


void divert_all_logs_to_file (const std::string& filename)
{
    log::add_common_attributes();
    log::register_simple_formatter_factory<riak::log::request_id_type, char>("Riak/ClientRequestId");
    log::register_simple_formatter_factory<riak::log::severity, char>("Severity");

    auto backend = boost::make_shared<log::sinks::text_file_backend>(log::keywords::file_name = filename);
    auto format_as_text = boost::make_shared<log::sinks::synchronous_sink<log::sinks::text_file_backend>>(backend);

    format_as_text->set_formatter( expr::stream
         << expr::if_ (channel == riak::log::channel::core || channel == riak::log::channel::network) [
                expr::stream
                    << "---- "
                    << "(riak) "
                    << expr::format_date_time<boost::posix_time::ptime>("TimeStamp", "%Y-%m-%d %H:%M:%S.%f ")
                    << severity << ' '
                    << '[' << request_id << ']'
            ] .else_ [
                expr::stream << "(test) [" << expr::attr<std::string>("TestCase") << "]"
            ]
         << ": " << expr::message
        );

    log::core::get()->add_sink(format_as_text);
}

//=============================================================================
}   // namespace (anonymous)
//=============================================================================

int main (int argc, char* argv[])
{
    ::testing::InitGoogleTest(&argc, argv);
    divert_all_logs_to_file("units.log");

    return RUN_ALL_TESTS();
}
