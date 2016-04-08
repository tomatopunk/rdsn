/**
 * Autogenerated by Thrift Compiler (0.9.3)
 *
 * DO NOT EDIT UNLESS YOU ARE SURE THAT YOU KNOW WHAT YOU ARE DOING
 *  @generated
 */
#ifndef cli_TYPES_H
#define cli_TYPES_H

#include <dsn/service_api_cpp.h>
#include <iosfwd>

#include <thrift/Thrift.h>
#include <thrift/TApplicationException.h>
#include <thrift/protocol/TProtocol.h>
#include <thrift/transport/TTransport.h>

#include <thrift/cxxfunctional.h>


namespace dsn {

class command;

typedef struct _command__isset {
  _command__isset() : cmd(false), arguments(false) {}
  bool cmd :1;
  bool arguments :1;
} _command__isset;

class command {
 public:

  command(const command&);
  command(command&&);
  command& operator=(const command&);
  command& operator=(command&&);
  command() : cmd() {
  }

  virtual ~command() throw();
  std::string cmd;
  std::vector<std::string>  arguments;

  _command__isset __isset;

  void __set_cmd(const std::string& val);

  void __set_arguments(const std::vector<std::string> & val);

  bool operator == (const command & rhs) const
  {
    if (!(cmd == rhs.cmd))
      return false;
    if (!(arguments == rhs.arguments))
      return false;
    return true;
  }
  bool operator != (const command &rhs) const {
    return !(*this == rhs);
  }

  bool operator < (const command & ) const;

  uint32_t read(::apache::thrift::protocol::TProtocol* iprot);
  uint32_t write(::apache::thrift::protocol::TProtocol* oprot) const;

  virtual void printTo(std::ostream& out) const;
};

void swap(command &a, command &b);

inline std::ostream& operator<<(std::ostream& out, const command& obj)
{
  obj.printTo(out);
  return out;
}

} // namespace

#endif