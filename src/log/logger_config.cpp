#include <fc/log/logger_config.hpp>
#include <fc/log/appender.hpp>
#include <fc/io/json.hpp>
#include <fc/filesystem.hpp>
#include <unordered_map>
#include <string>
#include <fc/log/console_appender.hpp>
#include <fc/log/file_appender.hpp>
#include <fc/log/gelf_appender.hpp>
#include <fc/reflect/variant.hpp>
#include <fc/exception/exception.hpp>
#include <fc/io/stdio.hpp>
#include <boost/algorithm/cxx11/copy_if.hpp>

namespace fc {

bool logging_config::configure_logging(const logging_config& cfg)
{
    try
    {
        logger::clear_all();
        appender::clear_all();

        // slog( "\n%s", fc::json::to_pretty_string(cfg).c_str() );
        for (size_t i = 0; i < cfg.appenders.size(); ++i)
        {
            appender::create(cfg.appenders[i].name, cfg.appenders[i].type, cfg.appenders[i].args);
            // TODO... process enabled
        }

        for (size_t i = 0; i < cfg.loggers.size(); ++i)
        {
            const auto& logger_cfg = cfg.loggers[i];
            auto logger = logger::get(logger_cfg.name);
            for (const auto& appender_name : logger_cfg.appenders)
            {
                auto appender = appender::get(appender_name);
                if (appender)
                {
                    logger.add_appender(logger_cfg.level, appender);
                }
            }
        }
        return true;
    }
    catch (exception& e)
    {
        fc::cerr << e.to_detail_string() << "\n";
    }
    return false;
}

logging_config logging_config::default_config()
{
    // slog( "default cfg" );
    logging_config cfg;

    variants c;
    c.push_back(mutable_variant_object("level", "debug")("color", "green"));
    c.push_back(mutable_variant_object("level", "warn")("color", "brown"));
    c.push_back(mutable_variant_object("level", "error")("color", "red"));

    // clang-format off
    cfg.appenders.push_back(fc::appender_config::create_config<fc::console_appender>("stderr", mutable_variant_object()("stream", "std_error")("level_colors", c)));
    cfg.appenders.push_back(fc::appender_config::create_config<fc::console_appender>("stdout", mutable_variant_object()("stream", "std_out")("level_colors", c)));
    // clang-format on

    logger_config dlc;
    dlc.name = "default";
    dlc.level = log_level::info;
    dlc.appenders.push_back("stderr");
    cfg.loggers.push_back(dlc);
    return cfg;
}

bool do_default_config = logging_config::configure_logging();
}
