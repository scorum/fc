#pragma once
#include <fc/log/logger.hpp>
#include <typeindex>

namespace fc {

   struct appender_config {

      string   name;
      std::type_index   type;
      variant  args;
      bool     enabled;

      template<typename T>
      static appender_config create_config(const string& name = "", variant args = variant())
      {
          return appender_config(typeid(T), name, args);
      }

   private:
      appender_config(const std::type_index& type , const string& name, variant args) :
          name(name),
          type(type),
          args(fc::move(args)),
          enabled(true)
      {}
   };

   struct logger_config {
      logger_config(const fc::string& name = ""):name(name),enabled(true),additivity(false){}
      string                           name;
      ostring                          parent;
      /// if not set, then parents level is used.
      fc::optional<log_level>          level;
      bool                             enabled;
      /// if any appenders are specified, then parent's appenders are not set.
      bool                             additivity;
      std::vector<string>              appenders;
   };

   struct logging_config {
      static logging_config default_config();
      static bool configure_logging(const logging_config& l = default_config());

      std::vector<appender_config> appenders;
      std::vector<logger_config>   loggers;
   };

   
}

#include <fc/reflect/reflect.hpp>
FC_REFLECT( fc::appender_config, (name)(type)(args)(enabled) )
FC_REFLECT( fc::logger_config, (name)(parent)(level)(enabled)(additivity)(appenders) )
FC_REFLECT( fc::logging_config, (appenders)(loggers) )

