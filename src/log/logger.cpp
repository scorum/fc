#include <fc/log/logger.hpp>
#include <fc/log/log_message.hpp>
#include <fc/thread/thread.hpp>
#include <fc/thread/spin_lock.hpp>
#include <fc/thread/scoped_lock.hpp>
#include <fc/log/appender.hpp>
#include <fc/filesystem.hpp>
#include <unordered_map>
#include <string>
#include <iostream>
#include <thread> 

namespace fc {

    static fc::spin_lock s_logger_spinlock;

    static fc::thread s_logger_thread = fc::thread("logger");

    class logger::impl : public fc::retainable
    {
        fc::spin_lock    _this_lock;

        log_level        _level;

        std::map<log_level::values, std::vector<appender::ptr>, std::greater<log_level::values>> _appenders;

      public:
        impl():_level(log_level::off){}

        void set_level(const log_level& l)
        {
            scoped_lock<spin_lock> lock(_this_lock);

            if (l < _level)
                _level = l;
        }

        const log_level get_level() const 
        {
            return _level;
        }

        void log(const log_level& log_level, const log_message& m)
        {
            auto most_strong_suitable_level_it = _appenders.lower_bound(log_level.value);
            for (auto it = most_strong_suitable_level_it; it != _appenders.end(); ++it)
                for (const auto& appender : it->second)
                    appender->log(m);
        }

        void add_appender(const log_level& l, const fc::shared_ptr<appender>& a)
        {
            set_level(l);

            _appenders[l.value].push_back(a);
        }
    };

    logger::logger()
    :_my( new impl() ){}

    logger::logger(nullptr_t){}

    logger::logger( const std::string& name)
    :_my( new impl() ){}

    logger::logger( const logger& l )
    :_my(l._my){}

    logger::logger( logger&& l )
    :_my(std::move(l._my)){}

    logger::~logger(){}

    logger& logger::operator=( const logger& l ){
       _my = l._my;
       return *this;
    }
    logger& logger::operator=( logger&& l ){
       fc_swap(_my,l._my);
       return *this;
    }
    bool operator==( const logger& l, std::nullptr_t ) { return !l._my; }
    bool operator!=( const logger& l, std::nullptr_t ) { return l._my;  }

    bool logger::is_enabled( log_level e )const
    {
       return e >= _my->get_level();
    }

    void logger::log(const log_level& l, const log_message& m)
    {
        auto impl = _my;//don't capture _my instead impl, it capture this implicitly
        s_logger_thread.async([=]() { impl->log(l, m); }, "logging");
    }

    void logger::add_appender(const log_level& l, const fc::shared_ptr<appender>& a)
    {
        _my->set_level(l);

        auto impl = _my;//don't capture _my instead impl, it capture this implicitly
        s_logger_thread.async([=]() { impl->add_appender(l, a); }, "add_appender");
    }

    std::unordered_map<std::string,logger>& get_logger_map()
    {
      //TODO: Atomic compare/swap set
      static std::unordered_map<std::string,logger> lm;
      return lm;
    }

    logger logger::get( const std::string& s )
    {
       scoped_lock<spin_lock> lock(s_logger_spinlock);
       return get_logger_map()[s];
    }

    void logger::clear_all()
    {
        scoped_lock<spin_lock> lock(s_logger_spinlock);
        get_logger_map().clear();
    }

} // namespace fc

