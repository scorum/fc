#include <fc/log/logger_config.hpp>
#include <fc/log/appender.hpp>
#include <fc/io/json.hpp>
#include <fc/filesystem.hpp>
#include <unordered_map>
#include <string>
#include <fc/log/console_appender.hpp>
#include <fc/log/file_appender.hpp>
#include <fc/reflect/variant.hpp>
#include <fc/exception/exception.hpp>
#include <fc/io/stdio.hpp>
#include <boost/algorithm/cxx11/copy_if.hpp>

namespace fc {
   extern std::unordered_map<std::string,logger>& get_logger_map();
   extern std::unordered_map<std::string,appender::ptr>& get_appender_map();
   logger_config& logger_config::add_appender( const string& s ) { appenders.push_back(s); return *this; }

   void configure_logging( const fc::path& lc )
   {
      configure_logging( fc::json::from_file<logging_config>(lc) );
   }
   bool configure_logging( const logging_config& cfg )
   {
      try {
      static bool reg_console_appender = appender::register_appender<console_appender>( "console" );
      static bool reg_file_appender = appender::register_appender<file_appender>( "file" );
      get_logger_map().clear();
      get_appender_map().clear();

      //slog( "\n%s", fc::json::to_pretty_string(cfg).c_str() );
      for( size_t i = 0; i < cfg.appenders.size(); ++i ) {
         appender::create( cfg.appenders[i].name, cfg.appenders[i].type, cfg.appenders[i].args );
        // TODO... process enabled
      }

      std::vector<logger_config> loggers;
      boost::algorithm::copy_if(cfg.loggers, std::back_inserter(loggers),
                                [](const logger_config& lhs) { return lhs.level.valid(); });
      for (size_t i = 0; i < loggers.size(); ++i)
      {
          const auto& logger_cfg = loggers[i];
          auto logger = logger::get(logger_cfg.name);
          logger.set_name(logger_cfg.name);
          logger.set_log_level(*logger_cfg.level);
          if (logger_cfg.parent.valid())
          {
              logger.set_parent(logger::get(*logger_cfg.parent));
          }
          for (const auto& appender_name : logger_cfg.appenders)
          {
              auto appender = appender::get(appender_name);
              if (appender)
              {
                  logger.add_appender(*logger_cfg.level, appender);
              }
          }
      }
      return reg_console_appender || reg_file_appender;
      } catch ( exception& e )
      {
         fc::cerr<<e.to_detail_string()<<"\n";
      }
      return false;
   }

   logging_config logging_config::default_config() {
      //slog( "default cfg" );
      logging_config cfg;

     variants  c;
               c.push_back(  mutable_variant_object( "level","debug")("color", "green") );
               c.push_back(  mutable_variant_object( "level","warn")("color", "brown") );
               c.push_back(  mutable_variant_object( "level","error")("color", "red") );

      cfg.appenders.push_back( 
             appender_config( "stderr", "console", 
                 mutable_variant_object()
                     ( "stream","std_error")
                     ( "level_colors", c ) 
                 ) ); 
      cfg.appenders.push_back( 
             appender_config( "stdout", "console", 
                 mutable_variant_object()
                     ( "stream","std_out") 
                     ( "level_colors", c ) 
                 ) ); 
      
      logger_config dlc;
      dlc.name = "default";
      dlc.level = log_level::debug;
      dlc.appenders.push_back("stderr");
      cfg.loggers.push_back( dlc );
      return cfg;
   }
}
