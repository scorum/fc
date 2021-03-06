#pragma once

#include <string>
#include <fc/time.hpp>
#include <fc/shared_ptr.hpp>
#include <fc/log/log_message.hpp>

namespace fc  
{

   class appender;

   /**
    *
    *
    @code
      void my_class::func() 
      {
         fc_dlog( my_class_logger, "Format four: ${arg}  five: ${five}", ("arg",4)("five",5) );
      }
    @endcode
    */
   class logger 
   {
      public:
    static logger get(const std::string& name = "default");
         static void   clear_all();

         logger();
         logger(const std::string& name);
         logger( std::nullptr_t );
         logger( const logger& c );
         logger( logger&& c );
         ~logger();
         logger& operator=(const logger&);
         logger& operator=(logger&&);
         friend bool operator==( const logger&, nullptr_t );
         friend bool operator!=( const logger&, nullptr_t );

         void add_appender( const log_level& log_level, const fc::shared_ptr<appender>& a );

         bool is_enabled( log_level e )const;
         void log(const log_level& log_level, const log_message& m);

      private:
         class impl;
         fc::shared_ptr<impl> _my;
   };

} // namespace fc

#ifndef DEFAULT_LOGGER
#define DEFAULT_LOGGER
#endif

// suppress warning "conditional expression is constant" in the while(0) for visual c++
// http://cnicholson.net/2009/03/stupid-c-tricks-dowhile0-and-c4127/
#define FC_MULTILINE_MACRO_BEGIN do {
#ifdef _MSC_VER
# define FC_MULTILINE_MACRO_END \
    __pragma(warning(push)) \
    __pragma(warning(disable:4127)) \
    } while (0) \
    __pragma(warning(pop))
#else
# define FC_MULTILINE_MACRO_END  } while (0)
#endif

#define FC_NAMED_LOG( LOGGER, LEVEL, OUTPUT ) \
  FC_MULTILINE_MACRO_BEGIN \
    auto logger = (LOGGER);                                                                                            \
    if (logger.is_enabled(LEVEL))                                                                                      \
        logger.log(LEVEL, OUTPUT);                                                                                     \
  FC_MULTILINE_MACRO_END

#define FC_DEFAULT_LOG( LOGGER, LEVEL, OUTPUT ) \
    FC_NAMED_LOG(fc::logger::get(DEFAULT_LOGGER), LEVEL, OUTPUT)

#define FC_NAMED_LOG_FORMATED( LOGGER, LEVEL, FORMAT, ... ) \
    FC_NAMED_LOG(LOGGER, fc::log_level::LEVEL, FC_LOG_MESSAGE(LEVEL, FORMAT, __VA_ARGS__))

#define FC_NAMED_LOG_CTX_FORMATED( LOGGER, LEVEL, CTX, FORMAT, ... ) \
    FC_NAMED_LOG(LOGGER, fc::log_level::LEVEL, FC_LOG_CTX_MESSAGE(LEVEL, CTX, FORMAT, __VA_ARGS__))

#define fc_dlog( LOGGER, FORMAT, ... ) \
    FC_NAMED_LOG_FORMATED(LOGGER, debug, FORMAT, __VA_ARGS__)

#define fc_ilog( LOGGER, FORMAT, ... ) \
    FC_NAMED_LOG_FORMATED(LOGGER, info, FORMAT, __VA_ARGS__)

#define fc_wlog( LOGGER, FORMAT, ... ) \
    FC_NAMED_LOG_FORMATED(LOGGER, warn, FORMAT, __VA_ARGS__)

#define fc_elog( LOGGER, FORMAT, ... ) \
    FC_NAMED_LOG_FORMATED(LOGGER, error, FORMAT, __VA_ARGS__)

#define fc_ctx_dlog( LOGGER, CTX, FORMAT, ... ) \
    FC_NAMED_LOG_CTX_FORMATED(LOGGER, debug, CTX, FORMAT, __VA_ARGS__)

#define fc_ctx_ilog( LOGGER, CTX, FORMAT, ... ) \
    FC_NAMED_LOG_CTX_FORMATED(LOGGER, info, CTX, FORMAT, __VA_ARGS__)

#define fc_ctx_wlog( LOGGER, CTX, FORMAT, ... ) \
    FC_NAMED_LOG_CTX_FORMATED(LOGGER, warn, CTX, FORMAT, __VA_ARGS__)

#define fc_ctx_elog( LOGGER, CTX, FORMAT, ... ) \
    FC_NAMED_LOG_CTX_FORMATED(LOGGER, error, CTX, FORMAT, __VA_ARGS__)

#define dlog( FORMAT, ... ) \
    fc_dlog(fc::logger::get(DEFAULT_LOGGER), FORMAT, __VA_ARGS__)

#define ilog( FORMAT, ... ) \
    fc_ilog(fc::logger::get(DEFAULT_LOGGER), FORMAT, __VA_ARGS__)

#define wlog( FORMAT, ... ) \
    fc_wlog(fc::logger::get(DEFAULT_LOGGER), FORMAT, __VA_ARGS__)

#define elog( FORMAT, ... ) \
    fc_elog(fc::logger::get(DEFAULT_LOGGER), FORMAT, __VA_ARGS__)

#define ctx_dlog( CTX, FORMAT, ... ) \
    fc_ctx_dlog(fc::logger::get(DEFAULT_LOGGER), CTX, FORMAT, __VA_ARGS__)

#define ctx_ilog( CTX, FORMAT, ... ) \
    fc_ctx_ilog(fc::logger::get(DEFAULT_LOGGER), CTX, FORMAT, __VA_ARGS__)

#define ctx_wlog( CTX, FORMAT, ... ) \
    fc_ctx_wlog(fc::logger::get(DEFAULT_LOGGER), CTX, FORMAT, __VA_ARGS__)

#define ctx_elog( CTX, FORMAT, ... ) \
    fc_ctx_elog(fc::logger::get(DEFAULT_LOGGER), CTX, FORMAT, __VA_ARGS__)

/**
 * Sends the log message to a special 'user' log stream designed for messages that
 * the end user may like to see.
 */
#define ulog( FORMAT, ... ) \
    fc_dlog(fc::logger::get("user"), FORMAT, __VA_ARGS__)

#include <boost/preprocessor/seq/for_each.hpp>
#include <boost/preprocessor/seq/enum.hpp>
#include <boost/preprocessor/seq/size.hpp>
#include <boost/preprocessor/seq/seq.hpp>
#include <boost/preprocessor/stringize.hpp>
#include <boost/preprocessor/punctuation/paren.hpp>


#define FC_FORMAT_ARG(r, unused, base) \
  BOOST_PP_STRINGIZE(base) ": ${" BOOST_PP_STRINGIZE( base ) "} "

#define FC_FORMAT_ARGS(r, unused, base) \
  BOOST_PP_LPAREN() BOOST_PP_STRINGIZE(base),fc::variant(base) BOOST_PP_RPAREN()

#define FC_FORMAT( SEQ )\
    BOOST_PP_SEQ_FOR_EACH( FC_FORMAT_ARG, v, SEQ ) 

// takes a ... instead of a SEQ arg because it can be called with an empty SEQ 
// from FC_CAPTURE_AND_THROW()
#define FC_FORMAT_ARG_PARAMS( ... )\
    BOOST_PP_SEQ_FOR_EACH( FC_FORMAT_ARGS, v, __VA_ARGS__ ) 

#define idump( SEQ ) \
    ilog( FC_FORMAT(SEQ), FC_FORMAT_ARG_PARAMS(SEQ) )  
#define wdump( SEQ ) \
    wlog( FC_FORMAT(SEQ), FC_FORMAT_ARG_PARAMS(SEQ) )  
#define edump( SEQ ) \
    elog( FC_FORMAT(SEQ), FC_FORMAT_ARG_PARAMS(SEQ) )  

// this disables all normal logging statements -- not something you'd normally want to do,
// but it's useful if you're benchmarking something and suspect logging is causing
// a slowdown.
#ifdef FC_DISABLE_LOGGING
# undef ulog
# define ulog(...) FC_MULTILINE_MACRO_BEGIN FC_MULTILINE_MACRO_END
# undef elog
# define elog(...) FC_MULTILINE_MACRO_BEGIN FC_MULTILINE_MACRO_END
# undef wlog
# define wlog(...) FC_MULTILINE_MACRO_BEGIN FC_MULTILINE_MACRO_END
# undef ilog
# define ilog(...) FC_MULTILINE_MACRO_BEGIN FC_MULTILINE_MACRO_END
# undef dlog
# define dlog(...) FC_MULTILINE_MACRO_BEGIN FC_MULTILINE_MACRO_END
#endif
