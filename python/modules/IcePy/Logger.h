//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef ICEPY_LOGGER_H
#define ICEPY_LOGGER_H

#include <Config.h>
#include <Util.h>
#include <Ice/Logger.h>

#include <memory>

namespace IcePy
{

//
// LoggerWrapper delegates to a Python implementation.
//
class LoggerWrapper : public Ice::Logger
{
public:

    LoggerWrapper(PyObject*);

    virtual void print(const std::string&);
    virtual void trace(const std::string&, const std::string&);
    virtual void warning(const std::string&);
    virtual void error(const std::string&);
    virtual std::string getPrefix();
    virtual Ice::LoggerPtr cloneWithPrefix(const std::string&);
    PyObject* getObject();

private:

    PyObjectHandle _logger;
};
using LoggerWrapperPtr = std::shared_ptr<LoggerWrapper>;

bool initLogger(PyObject*);

void cleanupLogger();

//
// Create a Python object that delegates to a C++ implementation.
//
PyObject* createLogger(const Ice::LoggerPtr&);

}

extern "C" PyObject* IcePy_getProcessLogger(PyObject*, PyObject*);
extern "C" PyObject* IcePy_setProcessLogger(PyObject*, PyObject*);

#endif
