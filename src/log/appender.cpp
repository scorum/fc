#include <fc/log/appender.hpp>
#include <fc/thread/unique_lock.hpp>
#include <unordered_map>
#include <string>
#include <sstream>
#include <iomanip>
#include <fc/thread/spin_lock.hpp>
#include <fc/thread/scoped_lock.hpp>
#include <fc/log/console_appender.hpp>
#include <fc/log/file_appender.hpp>
#include <fc/log/gelf_appender.hpp>
#include <fc/variant.hpp>
#include <fc/unused.hpp>
#include "console_defines.h"


namespace fc {

   std::unordered_map<std::string,appender::ptr>& get_appender_map() {
     static std::unordered_map<std::string,appender::ptr> lm;
     return lm;
   }
   std::unordered_map<std::type_index,appender_factory::ptr>& get_appender_factory_map() {
     static std::unordered_map<std::type_index,appender_factory::ptr> lm;
     return lm;
   }

   //////////////////////////////////////////////////////////////////////////
   appender::ptr appender::get( const fc::string& s ) {
     static fc::spin_lock appender_spinlock;
      scoped_lock<spin_lock> lock(appender_spinlock);
      return get_appender_map()[s];
   }

   bool  appender::register_factory(const std::type_index& type, const appender_factory::ptr& f )
   {
      get_appender_factory_map()[type] = f;
      return true;
   }

   appender::ptr appender::create( const fc::string& name, const std::type_index& type, const variant& args  )
   {
      auto fact_itr = get_appender_factory_map().find(type);
      if( fact_itr == get_appender_factory_map().end() ) {
         //wlog( "Unknown appender type '%s'", type.name() );
         return appender::ptr();
      }
      auto ap = fact_itr->second->create( args );
      get_appender_map()[name] = ap;
      return ap;
   }
   void appender::clear_all()
   {
       get_appender_map().clear();
   }

   std::string appender::default_format(const log_message& m)
   {
       std::stringstream file_line;
       file_line << m.get_context().get_file() << ":" << m.get_context().get_line_number() << " ";

       ///////////////
       std::stringstream line;
       line << string(m.get_context().get_timestamp()) << " ";
       line << std::setw(6) << std::left << m.get_context().get_thread_name().substr(0, 5) << " ";
       line << std::setw(30) << std::left << file_line.str();

       string method_name = m.get_context().get_method();
       // strip all leading scopes...
       if (method_name.size())
       {
           uint32_t p = 0;
           for (uint32_t i = 0; i < method_name.size(); ++i)
           {
               if (method_name[i] == ':') p = i;
           }

           if (method_name[p] == ':')
               ++p;
           line << std::setw(20) << std::left << m.get_context().get_method().substr(p, 20) << " ";
       }

       line << "] ";
       const std::string& context_str = m.get_context().get_context();
       if (!context_str.empty())
       {
           line << "(" << context_str << ") ";
       }
       line << fc::format_string(m.get_format(), m.get_data());

       if (!m.get_context().get_task_name().empty())
            line << "\t\t\t(task: " << m.get_context().get_task_name() << ")";

       line << std::endl;

       return line.str();
   }

   FC_UNUSED static bool reg_console_appender = appender::create_appender_factory<console_appender>();
   FC_UNUSED static bool reg_file_appender = appender::create_appender_factory<file_appender>();
   FC_UNUSED static bool reg_gelf_appender = appender::create_appender_factory<gelf_appender>();

} // namespace fc
