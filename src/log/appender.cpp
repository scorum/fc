#include <fc/log/appender.hpp>
#include <fc/thread/unique_lock.hpp>
#include <unordered_map>
#include <string>
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

   FC_UNUSED static bool reg_console_appender = appender::create_appender_factory<console_appender>();
   FC_UNUSED static bool reg_file_appender = appender::create_appender_factory<file_appender>();
   FC_UNUSED static bool reg_gelf_appender = appender::create_appender_factory<gelf_appender>();

} // namespace fc
