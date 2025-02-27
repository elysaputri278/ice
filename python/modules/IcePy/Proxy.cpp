//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include <IceUtil/DisableWarnings.h>
#include <Proxy.h>
#include <structmember.h>
#include <Communicator.h>
#include <Connection.h>
#include <Endpoint.h>
#include <Operation.h>
#include <Thread.h>
#include <Util.h>
#include <Types.h>
#include <Ice/Communicator.h>
#include <Ice/LocalException.h>
#include <Ice/Locator.h>
#include <Ice/Proxy.h>
#include <Ice/Router.h>

using namespace std;
using namespace IcePy;

#if defined(__GNUC__) && ((__GNUC__ >= 8))
#   pragma GCC diagnostic ignored "-Wcast-function-type"
#endif

namespace IcePy
{

struct ProxyObject
{
    PyObject_HEAD
    Ice::ObjectPrx* proxy;
    Ice::CommunicatorPtr* communicator;
};

}

//
// Proxy implementation.
//
static ProxyObject*
allocateProxy(const Ice::ObjectPrx& proxy, const Ice::CommunicatorPtr& communicator, PyObject* type)
{
    PyTypeObject* typeObj = reinterpret_cast<PyTypeObject*>(type);
    ProxyObject* p = reinterpret_cast<ProxyObject*>(typeObj->tp_alloc(typeObj, 0));
    if(!p)
    {
        return 0;
    }

    //
    // Disabling collocation optimization can cause subtle problems with proxy
    // comparison (such as in RouterInfo::get) if a proxy from IcePy is
    // compared with a proxy from Ice/C++.
    //
    //if(proxy)
    //{
    //    p->proxy = new Ice::ObjectPrx(proxy->ice_collocationOptimized(false));
    //}
    //
    p->proxy = new Ice::ObjectPrx(proxy);
    p->communicator = new Ice::CommunicatorPtr(communicator);

    return p;
}

#ifdef WIN32
extern "C"
#endif
static ProxyObject*
proxyNew(PyTypeObject* /*type*/, PyObject* /*args*/, PyObject* /*kwds*/)
{
    PyErr_Format(PyExc_RuntimeError, STRCAST("A proxy cannot be created directly"));
    return 0;
}

#ifdef WIN32
extern "C"
#endif
static void
proxyDealloc(ProxyObject* self)
{
    delete self->proxy;
    delete self->communicator;
    Py_TYPE(self)->tp_free(reinterpret_cast<PyObject*>(self));
}

#ifdef WIN32
extern "C"
#endif
static PyObject*
proxyCompare(ProxyObject* p1, PyObject* other, int op)
{
    bool result = false;

    if(PyObject_TypeCheck(other, &ProxyType))
    {
        ProxyObject* p2 = reinterpret_cast<ProxyObject*>(other);

        switch(op)
        {
        case Py_EQ:
            result = *p1->proxy == *p2->proxy;
            break;
        case Py_NE:
            result = *p1->proxy != *p2->proxy;
            break;
        case Py_LE:
            result = *p1->proxy <= *p2->proxy;
            break;
        case Py_GE:
            result = *p1->proxy >= *p2->proxy;
            break;
        case Py_LT:
            result = *p1->proxy < *p2->proxy;
            break;
        case Py_GT:
            result = *p1->proxy > *p2->proxy;
            break;
        }
    }
    else if(other == Py_None)
    {
        result = op == Py_NE || op == Py_GE || op == Py_GT;
    }
    else
    {
        if(op == Py_EQ)
        {
            result = false;
        }
        else if(op == Py_NE)
        {
            result = true;
        }
        else
        {
            PyErr_Format(PyExc_TypeError, "can't compare %s to %s", Py_TYPE(p1)->tp_name, Py_TYPE(other)->tp_name);
            return 0;
        }
    }

    return result ? incTrue() : incFalse();
}

#ifdef WIN32
extern "C"
#endif
static PyObject*
proxyRepr(ProxyObject* self, PyObject* /*args*/)
{
    string str = (*self->proxy)->ice_toString();
    return createString(str);
}

#ifdef WIN32
extern "C"
#endif
static long
proxyHash(ProxyObject* self)
{
    return static_cast<long>((*self->proxy)->_hash());
}

#ifdef WIN32
extern "C"
#endif
static PyObject*
proxyIceGetCommunicator(ProxyObject* self, PyObject* /*args*/)
{
    return getCommunicatorWrapper(*self->communicator);
}

#ifdef WIN32
extern "C"
#endif
static PyObject*
proxyIceIsA(ProxyObject* self, PyObject* args)
{
    PyObject* type;
    PyObject* ctx = Py_None;
    if(!PyArg_ParseTuple(args, STRCAST("O|O!"), &type, &PyDict_Type, &ctx))
    {
        return 0;
    }

    //
    // We need to reformat the arguments to match what is used by the generated code: ((params...), ctx|None)
    //
    PyObjectHandle newArgs = Py_BuildValue(STRCAST("((O), O)"), type, ctx);

    return invokeBuiltin(reinterpret_cast<PyObject*>(self), "ice_isA", newArgs.get());
}

#ifdef WIN32
extern "C"
#endif
static PyObject*
proxyIceIsAAsync(ProxyObject* self, PyObject* args)
{
    PyObject* type;
    PyObject* ctx = Py_None;
    if(!PyArg_ParseTuple(args, STRCAST("O|O!"), &type, &PyDict_Type, &ctx))
    {
        return 0;
    }

    //
    // We need to reformat the arguments to match what is used by the generated code: ((params...), ctx|None)
    //
    PyObjectHandle newArgs = Py_BuildValue(STRCAST("((O), O)"), type, ctx);

    return invokeBuiltinAsync(reinterpret_cast<PyObject*>(self), "ice_isA", newArgs.get());
}

#ifdef WIN32
extern "C"
#endif
static PyObject*
proxyIcePing(ProxyObject* self, PyObject* args)
{
    PyObject* ctx = Py_None;
    if(!PyArg_ParseTuple(args, STRCAST("|O!"), &PyDict_Type, &ctx))
    {
        return 0;
    }

    //
    // We need to reformat the arguments to match what is used by the generated code: ((params...), ctx|None)
    //
    PyObjectHandle newArgs = Py_BuildValue(STRCAST("((), O)"), ctx);

    return invokeBuiltin(reinterpret_cast<PyObject*>(self), "ice_ping", newArgs.get());
}

#ifdef WIN32
extern "C"
#endif
static PyObject*
proxyIcePingAsync(ProxyObject* self, PyObject* args)
{
    PyObject* ctx = Py_None;
    if(!PyArg_ParseTuple(args, STRCAST("|O!"), &PyDict_Type, &ctx))
    {
        return 0;
    }

    //
    // We need to reformat the arguments to match what is used by the generated code: ((params...), ctx|None)
    //
    PyObjectHandle newArgs = Py_BuildValue(STRCAST("((), O)"), ctx);

    return invokeBuiltinAsync(reinterpret_cast<PyObject*>(self), "ice_ping", newArgs.get());
}

#ifdef WIN32
extern "C"
#endif
static PyObject*
proxyIceIds(ProxyObject* self, PyObject* args)
{
    PyObject* ctx = Py_None;
    if(!PyArg_ParseTuple(args, STRCAST("|O!"), &PyDict_Type, &ctx))
    {
        return 0;
    }

    //
    // We need to reformat the arguments to match what is used by the generated code: ((params...), ctx|None)
    //
    PyObjectHandle newArgs = Py_BuildValue(STRCAST("((), O)"), ctx);

    return invokeBuiltin(reinterpret_cast<PyObject*>(self), "ice_ids", newArgs.get());
}

#ifdef WIN32
extern "C"
#endif
static PyObject*
proxyIceIdsAsync(ProxyObject* self, PyObject* args)
{
    PyObject* ctx = Py_None;
    if(!PyArg_ParseTuple(args, STRCAST("|O!"), &PyDict_Type, &ctx))
    {
        return 0;
    }

    //
    // We need to reformat the arguments to match what is used by the generated code: ((params...), ctx|None)
    //
    PyObjectHandle newArgs = Py_BuildValue(STRCAST("((), O)"), ctx);

    return invokeBuiltinAsync(reinterpret_cast<PyObject*>(self), "ice_ids", newArgs.get());
}

#ifdef WIN32
extern "C"
#endif
static PyObject*
proxyIceId(ProxyObject* self, PyObject* args)
{
    PyObject* ctx = Py_None;
    if(!PyArg_ParseTuple(args, STRCAST("|O!"), &PyDict_Type, &ctx))
    {
        return 0;
    }

    //
    // We need to reformat the arguments to match what is used by the generated code: ((params...), ctx|None)
    //
    PyObjectHandle newArgs = Py_BuildValue(STRCAST("((), O)"), ctx);

    return invokeBuiltin(reinterpret_cast<PyObject*>(self), "ice_id", newArgs.get());
}

#ifdef WIN32
extern "C"
#endif
static PyObject*
proxyIceIdAsync(ProxyObject* self, PyObject* args)
{
    PyObject* ctx = Py_None;
    if(!PyArg_ParseTuple(args, STRCAST("|O!"), &PyDict_Type, &ctx))
    {
        return 0;
    }

    //
    // We need to reformat the arguments to match what is used by the generated code: ((params...), ctx|None)
    //
    PyObjectHandle newArgs = Py_BuildValue(STRCAST("((), O)"), ctx);

    return invokeBuiltinAsync(reinterpret_cast<PyObject*>(self), "ice_id", newArgs.get());
}

#ifdef WIN32
extern "C"
#endif
static PyObject*
proxyIceGetIdentity(ProxyObject* self, PyObject* /*args*/)
{
    assert(self->proxy);

    Ice::Identity id;
    try
    {
        id = (*self->proxy)->ice_getIdentity();
    }
    catch(const Ice::Exception& ex)
    {
        setPythonException(ex);
        return 0;
    }

    return createIdentity(id);
}

#ifdef WIN32
extern "C"
#endif
static PyObject*
proxyIceIdentity(ProxyObject* self, PyObject* args)
{
    PyObject* identityType = lookupType("Ice.Identity");
    assert(identityType);
    PyObject* id;
    if(!PyArg_ParseTuple(args, STRCAST("O!"), identityType, &id))
    {
        return 0;
    }

    assert(self->proxy);

    Ice::Identity ident;
    if(!getIdentity(id, ident))
    {
        return 0;
    }

    optional<Ice::ObjectPrx> newProxy;
    try
    {
        newProxy = (*self->proxy)->ice_identity(ident);
    }
    catch(const Ice::Exception& ex)
    {
        setPythonException(ex);
        return 0;
    }

    return createProxy(newProxy.value(), *self->communicator);
}

#ifdef WIN32
extern "C"
#endif
static PyObject*
proxyIceGetContext(ProxyObject* self, PyObject* /*args*/)
{
    assert(self->proxy);

    Ice::Context ctx;
    try
    {
        ctx = (*self->proxy)->ice_getContext();
    }
    catch(const Ice::Exception& ex)
    {
        setPythonException(ex);
        return 0;
    }

    PyObjectHandle result = PyDict_New();
    if(result.get() && contextToDictionary(ctx, result.get()))
    {
        return result.release();
    }
    return 0;
}

#ifdef WIN32
extern "C"
#endif
static PyObject*
proxyIceContext(ProxyObject* self, PyObject* args)
{
    PyObject* dict;
    if(!PyArg_ParseTuple(args, STRCAST("O!"), &PyDict_Type, &dict))
    {
        return 0;
    }

    assert(self->proxy);

    Ice::Context ctx;
    if(!dictionaryToContext(dict, ctx))
    {
        return 0;
    }

    optional<Ice::ObjectPrx> newProxy;
    try
    {
        newProxy = (*self->proxy)->ice_context(ctx);
    }
    catch(const Ice::Exception& ex)
    {
        setPythonException(ex);
        return 0;
    }

    return createProxy(newProxy.value(), *self->communicator, reinterpret_cast<PyObject*>(Py_TYPE(self)));
}

#ifdef WIN32
extern "C"
#endif
static PyObject*
proxyIceGetFacet(ProxyObject* self, PyObject* /*args*/)
{
    assert(self->proxy);

    string facet;
    try
    {
        facet = (*self->proxy)->ice_getFacet();
    }
    catch(const Ice::Exception& ex)
    {
        setPythonException(ex);
        return 0;
    }

    return createString(facet);
}

#ifdef WIN32
extern "C"
#endif
static PyObject*
proxyIceFacet(ProxyObject* self, PyObject* args)
{
    PyObject* facetObj;
    if(!PyArg_ParseTuple(args, STRCAST("O"), &facetObj))
    {
        return 0;
    }

    string facet;
    if(!getStringArg(facetObj, "facet", facet))
    {
        return 0;
    }

    assert(self->proxy);

    optional<Ice::ObjectPrx> newProxy;
    try
    {
        newProxy = (*self->proxy)->ice_facet(facet);
    }
    catch(const Ice::Exception& ex)
    {
        setPythonException(ex);
        return 0;
    }

    return createProxy(newProxy.value(), *self->communicator);
}

#ifdef WIN32
extern "C"
#endif
static PyObject*
proxyIceGetAdapterId(ProxyObject* self, PyObject* /*args*/)
{
    assert(self->proxy);

    string id;
    try
    {
        id = (*self->proxy)->ice_getAdapterId();
    }
    catch(const Ice::Exception& ex)
    {
        setPythonException(ex);
        return 0;
    }

    return createString(id);
}

#ifdef WIN32
extern "C"
#endif
static PyObject*
proxyIceAdapterId(ProxyObject* self, PyObject* args)
{
    PyObject* idObj;
    if(!PyArg_ParseTuple(args, STRCAST("O"), &idObj))
    {
        return 0;
    }

    string id;
    if(!getStringArg(idObj, "id", id))
    {
        return 0;
    }

    assert(self->proxy);

    optional<Ice::ObjectPrx> newProxy;
    try
    {
        newProxy = (*self->proxy)->ice_adapterId(id);
    }
    catch(const Ice::Exception& ex)
    {
        setPythonException(ex);
        return 0;
    }

    return createProxy(newProxy.value(), *self->communicator, reinterpret_cast<PyObject*>(Py_TYPE(self)));
}

#ifdef WIN32
extern "C"
#endif
static PyObject*
proxyIceGetEndpoints(ProxyObject* self, PyObject* /*args*/)
{
    assert(self->proxy);

    Ice::EndpointSeq endpoints;
    try
    {
        endpoints = (*self->proxy)->ice_getEndpoints();
    }
    catch(const Ice::Exception& ex)
    {
        setPythonException(ex);
        return 0;
    }

    int count = static_cast<int>(endpoints.size());
    PyObjectHandle result = PyTuple_New(count);
    int i = 0;
    for(Ice::EndpointSeq::const_iterator p = endpoints.begin(); p != endpoints.end(); ++p, ++i)
    {
        PyObjectHandle endp = createEndpoint(*p);
        if(!endp.get())
        {
            return 0;
        }
        PyTuple_SET_ITEM(result.get(), i, endp.release()); // PyTuple_SET_ITEM steals a reference.
    }

    return result.release();
}

#ifdef WIN32
extern "C"
#endif
static PyObject*
proxyIceEndpoints(ProxyObject* self, PyObject* args)
{
    PyObject* endpoints;
    if(!PyArg_ParseTuple(args, STRCAST("O"), &endpoints))
    {
        return 0;
    }

    if(!PyTuple_Check(endpoints) && !PyList_Check(endpoints))
    {
        PyErr_Format(PyExc_ValueError, STRCAST("argument must be a tuple or list"));
        return 0;
    }

    assert(self->proxy);

    Ice::EndpointSeq seq;
    if(!toEndpointSeq(endpoints, seq))
    {
        return 0;
    }

    optional<Ice::ObjectPrx> newProxy;
    try
    {
        newProxy = (*self->proxy)->ice_endpoints(seq);
    }
    catch(const Ice::Exception& ex)
    {
        setPythonException(ex);
        return 0;
    }

    return createProxy(newProxy.value(), *self->communicator, reinterpret_cast<PyObject*>(Py_TYPE(self)));
}

#ifdef WIN32
extern "C"
#endif
static PyObject*
proxyIceGetLocatorCacheTimeout(ProxyObject* self, PyObject* /*args*/)
{
    assert(self->proxy);

    try
    {
        int32_t timeout = (*self->proxy)->ice_getLocatorCacheTimeout();
        return PyLong_FromLong(timeout);
    }
    catch(const Ice::Exception& ex)
    {
        setPythonException(ex);
        return 0;
    }
}

#ifdef WIN32
extern "C"
#endif
static PyObject*
proxyIceGetInvocationTimeout(ProxyObject* self, PyObject* /*args*/)
{
    assert(self->proxy);

    try
    {
        int32_t timeout = (*self->proxy)->ice_getInvocationTimeout();
        return PyLong_FromLong(timeout);
    }
    catch(const Ice::Exception& ex)
    {
        setPythonException(ex);
        return 0;
    }
}

#ifdef WIN32
extern "C"
#endif
static PyObject*
proxyIceGetConnectionId(ProxyObject* self, PyObject* /*args*/)
{
    assert(self->proxy);

    try
    {
        string connectionId = (*self->proxy)->ice_getConnectionId();
        return createString(connectionId);
    }
    catch(const Ice::Exception& ex)
    {
        setPythonException(ex);
        return 0;
    }
}

#ifdef WIN32
extern "C"
#endif
static PyObject*
proxyIceLocatorCacheTimeout(ProxyObject* self, PyObject* args)
{
    int timeout;
    if(!PyArg_ParseTuple(args, STRCAST("i"), &timeout))
    {
        return 0;
    }

    assert(self->proxy);

    optional<Ice::ObjectPrx> newProxy;
    try
    {
        newProxy = (*self->proxy)->ice_locatorCacheTimeout(timeout);
    }
    catch(const invalid_argument& ex)
    {
        PyErr_Format(PyExc_RuntimeError, "%s", ex.what());
        return 0;
    }
    catch(const Ice::Exception& ex)
    {
        setPythonException(ex);
        return 0;
    }

    return createProxy(newProxy.value(), *self->communicator, reinterpret_cast<PyObject*>(Py_TYPE(self)));
}

#ifdef WIN32
extern "C"
#endif
static PyObject*
proxyIceInvocationTimeout(ProxyObject* self, PyObject* args)
{
    int timeout;
    if(!PyArg_ParseTuple(args, STRCAST("i"), &timeout))
    {
        return 0;
    }

    assert(self->proxy);

    optional<Ice::ObjectPrx> newProxy;
    try
    {
        newProxy = (*self->proxy)->ice_invocationTimeout(timeout);
    }
    catch(const invalid_argument& ex)
    {
        PyErr_Format(PyExc_RuntimeError, "%s", ex.what());
        return 0;
    }
    catch(const Ice::Exception& ex)
    {
        setPythonException(ex);
        return 0;
    }

    return createProxy(newProxy.value(), *self->communicator, reinterpret_cast<PyObject*>(Py_TYPE(self)));
}

#ifdef WIN32
extern "C"
#endif
static PyObject*
proxyIceIsConnectionCached(ProxyObject* self, PyObject* /*args*/)
{
    assert(self->proxy);

    PyObject* b;
    try
    {
        b = (*self->proxy)->ice_isConnectionCached() ? getTrue() : getFalse();
    }
    catch(const Ice::Exception& ex)
    {
        setPythonException(ex);
        return 0;
    }

    Py_INCREF(b);
    return b;
}

#ifdef WIN32
extern "C"
#endif
static PyObject*
proxyIceConnectionCached(ProxyObject* self, PyObject* args)
{
    PyObject* flag;
    if(!PyArg_ParseTuple(args, STRCAST("O"), &flag))
    {
        return 0;
    }

    int n = PyObject_IsTrue(flag);
    if(n < 0)
    {
        return 0;
    }

    assert(self->proxy);

    optional<Ice::ObjectPrx> newProxy;
    try
    {
        newProxy = (*self->proxy)->ice_connectionCached(n == 1);
    }
    catch(const Ice::Exception& ex)
    {
        setPythonException(ex);
        return 0;
    }

    return createProxy(newProxy.value(), *self->communicator, reinterpret_cast<PyObject*>(Py_TYPE(self)));
}

#ifdef WIN32
extern "C"
#endif
static PyObject*
proxyIceGetEndpointSelection(ProxyObject* self,  PyObject* /*args*/)
{
    PyObject* cls = lookupType("Ice.EndpointSelectionType");
    assert(cls);

    PyObjectHandle rnd = getAttr(cls, "Random", false);
    PyObjectHandle ord = getAttr(cls, "Ordered", false);
    assert(rnd.get());
    assert(ord.get());

    assert(self->proxy);

    PyObject* type;
    try
    {
        Ice::EndpointSelectionType val = (*self->proxy)->ice_getEndpointSelection();
        if(val == Ice::EndpointSelectionType::Random)
        {
            type = rnd.get();
        }
        else
        {
            type = ord.get();
        }
    }
    catch(const Ice::Exception& ex)
    {
        setPythonException(ex);
        return 0;
    }

    Py_INCREF(type);
    return type;
}

#ifdef WIN32
extern "C"
#endif
static PyObject*
proxyIceEndpointSelection(ProxyObject* self, PyObject* args)
{
    PyObject* cls = lookupType("Ice.EndpointSelectionType");
    assert(cls);
    PyObject* type;
    if(!PyArg_ParseTuple(args, STRCAST("O!"), cls, &type))
    {
        return 0;
    }

    Ice::EndpointSelectionType val;
    PyObjectHandle rnd = getAttr(cls, "Random", false);
    PyObjectHandle ord = getAttr(cls, "Ordered", false);
    assert(rnd.get());
    assert(ord.get());
    if(rnd.get() == type)
    {
        val = Ice::EndpointSelectionType::Random;
    }
    else if(ord.get() == type)
    {
        val = Ice::EndpointSelectionType::Ordered;
    }
    else
    {
        PyErr_Format(PyExc_ValueError, STRCAST("ice_endpointSelection requires Random or Ordered"));
        return 0;
    }

    assert(self->proxy);

    optional<Ice::ObjectPrx> newProxy;
    try
    {
        newProxy = (*self->proxy)->ice_endpointSelection(val);
    }
    catch(const Ice::Exception& ex)
    {
        setPythonException(ex);
        return 0;
    }

    return createProxy(newProxy.value(), *self->communicator, reinterpret_cast<PyObject*>(Py_TYPE(self)));
}

#ifdef WIN32
extern "C"
#endif
static PyObject*
proxyIceIsSecure(ProxyObject* self, PyObject* /*args*/)
{
    assert(self->proxy);

    PyObject* b;
    try
    {
        b = (*self->proxy)->ice_isSecure() ? getTrue() : getFalse();
    }
    catch(const Ice::Exception& ex)
    {
        setPythonException(ex);
        return 0;
    }

    Py_INCREF(b);
    return b;
}

#ifdef WIN32
extern "C"
#endif
static PyObject*
proxyIceSecure(ProxyObject* self, PyObject* args)
{
    PyObject* flag;
    if(!PyArg_ParseTuple(args, STRCAST("O"), &flag))
    {
        return 0;
    }

    int n = PyObject_IsTrue(flag);
    if(n < 0)
    {
        return 0;
    }

    assert(self->proxy);

    optional<Ice::ObjectPrx> newProxy;
    try
    {
        newProxy = (*self->proxy)->ice_secure(n == 1);
    }
    catch(const Ice::Exception& ex)
    {
        setPythonException(ex);
        return 0;
    }

    return createProxy(newProxy.value(), *self->communicator, reinterpret_cast<PyObject*>(Py_TYPE(self)));
}

#ifdef WIN32
extern "C"
#endif
static PyObject*
proxyIceGetEncodingVersion(ProxyObject* self, PyObject* /*args*/)
{
    assert(self->proxy);

    PyObject* version;
    try
    {
        version = IcePy::createEncodingVersion((*self->proxy)->ice_getEncodingVersion());
    }
    catch(const Ice::Exception& ex)
    {
        setPythonException(ex);
        return 0;
    }

    Py_INCREF(version);
    return version;
}

#ifdef WIN32
extern "C"
#endif
static PyObject*
proxyIceEncodingVersion(ProxyObject* self, PyObject* args)
{
    PyObject* versionType = IcePy::lookupType("Ice.EncodingVersion");
    PyObject* p;
    if(!PyArg_ParseTuple(args, STRCAST("O!"), versionType, &p))
    {
        return 0;
    }

    Ice::EncodingVersion val;
    if(!getEncodingVersion(p, val))
    {
        PyErr_Format(PyExc_ValueError, STRCAST("ice_encodingVersion requires an encoding version"));
        return 0;
    }

    assert(self->proxy);

    optional<Ice::ObjectPrx> newProxy;
    try
    {
        newProxy = (*self->proxy)->ice_encodingVersion(val);
    }
    catch(const Ice::Exception& ex)
    {
        setPythonException(ex);
        return 0;
    }

    return createProxy(newProxy.value(), *self->communicator, reinterpret_cast<PyObject*>(Py_TYPE(self)));
}

#ifdef WIN32
extern "C"
#endif
static PyObject*
proxyIceIsPreferSecure(ProxyObject* self, PyObject* /*args*/)
{
    assert(self->proxy);

    PyObject* b;
    try
    {
        b = (*self->proxy)->ice_isPreferSecure() ? getTrue() : getFalse();
    }
    catch(const Ice::Exception& ex)
    {
        setPythonException(ex);
        return 0;
    }

    Py_INCREF(b);
    return b;
}

#ifdef WIN32
extern "C"
#endif
static PyObject*
proxyIcePreferSecure(ProxyObject* self, PyObject* args)
{
    PyObject* flag;
    if(!PyArg_ParseTuple(args, STRCAST("O"), &flag))
    {
        return 0;
    }

    int n = PyObject_IsTrue(flag);
    if(n < 0)
    {
        return 0;
    }

    assert(self->proxy);

    optional<Ice::ObjectPrx> newProxy;
    try
    {
        newProxy = (*self->proxy)->ice_preferSecure(n == 1);
    }
    catch(const Ice::Exception& ex)
    {
        setPythonException(ex);
        return 0;
    }

    return createProxy(newProxy.value(), *self->communicator, reinterpret_cast<PyObject*>(Py_TYPE(self)));
}

#ifdef WIN32
extern "C"
#endif
static PyObject*
proxyIceGetRouter(ProxyObject* self, PyObject* /*args*/)
{
    assert(self->proxy);

    optional<Ice::RouterPrx> router;
    try
    {
        router = (*self->proxy)->ice_getRouter();
    }
    catch(const Ice::Exception& ex)
    {
        setPythonException(ex);
        return 0;
    }

    if(!router)
    {
        Py_INCREF(Py_None);
        return Py_None;
    }

    PyObject* routerProxyType = lookupType("Ice.RouterPrx");
    assert(routerProxyType);
    return createProxy(router.value(), *self->communicator, routerProxyType);
}

#ifdef WIN32
extern "C"
#endif
static PyObject*
proxyIceRouter(ProxyObject* self, PyObject* args)
{
    PyObject* p;
    if(!PyArg_ParseTuple(args, STRCAST("O"), &p))
    {
        return 0;
    }

    optional<Ice::ObjectPrx> proxy;
    if(!getProxyArg(p, "ice_router", "rtr", proxy, "Ice.RouterPrx"))
    {
        return 0;
    }

    optional<Ice::RouterPrx> router = Ice::uncheckedCast<Ice::RouterPrx>(proxy);

    assert(self->proxy);

    optional<Ice::ObjectPrx> newProxy;
    try
    {
        newProxy = (*self->proxy)->ice_router(router);
    }
    catch(const Ice::Exception& ex)
    {
        setPythonException(ex);
        return 0;
    }

    return createProxy(newProxy.value(), *self->communicator, reinterpret_cast<PyObject*>(Py_TYPE(self)));
}

#ifdef WIN32
extern "C"
#endif
static PyObject*
proxyIceGetLocator(ProxyObject* self, PyObject* /*args*/)
{
    assert(self->proxy);

    optional<Ice::LocatorPrx> locator;
    try
    {
        locator = (*self->proxy)->ice_getLocator();
    }
    catch(const Ice::Exception& ex)
    {
        setPythonException(ex);
        return 0;
    }

    if(!locator)
    {
        Py_INCREF(Py_None);
        return Py_None;
    }

    PyObject* locatorProxyType = lookupType("Ice.LocatorPrx");
    assert(locatorProxyType);
    return createProxy(locator.value(), *self->communicator, locatorProxyType);
}

#ifdef WIN32
extern "C"
#endif
static PyObject*
proxyIceLocator(ProxyObject* self, PyObject* args)
{
    PyObject* p;
    if(!PyArg_ParseTuple(args, STRCAST("O"), &p))
    {
        return 0;
    }

    optional<Ice::ObjectPrx> proxy;
    if(!getProxyArg(p, "ice_locator", "loc", proxy, "Ice.LocatorPrx"))
    {
        return 0;
    }

    optional<Ice::LocatorPrx> locator = Ice::uncheckedCast<Ice::LocatorPrx>(proxy);

    assert(self->proxy);

    optional<Ice::ObjectPrx> newProxy;
    try
    {
        newProxy = (*self->proxy)->ice_locator(locator);
    }
    catch(const Ice::Exception& ex)
    {
        setPythonException(ex);
        return 0;
    }

    return createProxy(newProxy.value(), *self->communicator, reinterpret_cast<PyObject*>(Py_TYPE(self)));
}

#ifdef WIN32
extern "C"
#endif
static PyObject*
proxyIceTwoway(ProxyObject* self, PyObject* /*args*/)
{
    assert(self->proxy);

    optional<Ice::ObjectPrx> newProxy;
    try
    {
        newProxy = (*self->proxy)->ice_twoway();
    }
    catch(const Ice::Exception& ex)
    {
        setPythonException(ex);
        return 0;
    }

    return createProxy(newProxy.value(), *self->communicator, reinterpret_cast<PyObject*>(Py_TYPE(self)));
}

#ifdef WIN32
extern "C"
#endif
static PyObject*
proxyIceIsTwoway(ProxyObject* self, PyObject* /*args*/)
{
    assert(self->proxy);

    PyObject* b;
    try
    {
        b = (*self->proxy)->ice_isTwoway() ? getTrue() : getFalse();
    }
    catch(const Ice::Exception& ex)
    {
        setPythonException(ex);
        return 0;
    }

    Py_INCREF(b);
    return b;
}

#ifdef WIN32
extern "C"
#endif
static PyObject*
proxyIceOneway(ProxyObject* self, PyObject* /*args*/)
{
    assert(self->proxy);

    optional<Ice::ObjectPrx> newProxy;
    try
    {
        newProxy = (*self->proxy)->ice_oneway();
    }
    catch(const Ice::Exception& ex)
    {
        setPythonException(ex);
        return 0;
    }

    return createProxy(newProxy.value(), *self->communicator, reinterpret_cast<PyObject*>(Py_TYPE(self)));
}

#ifdef WIN32
extern "C"
#endif
static PyObject*
proxyIceIsOneway(ProxyObject* self, PyObject* /*args*/)
{
    assert(self->proxy);

    PyObject* b;
    try
    {
        b = (*self->proxy)->ice_isOneway() ? getTrue() : getFalse();
    }
    catch(const Ice::Exception& ex)
    {
        setPythonException(ex);
        return 0;
    }

    Py_INCREF(b);
    return b;
}

#ifdef WIN32
extern "C"
#endif
static PyObject*
proxyIceBatchOneway(ProxyObject* self, PyObject* /*args*/)
{
    assert(self->proxy);

    optional<Ice::ObjectPrx> newProxy;
    try
    {
        newProxy = (*self->proxy)->ice_batchOneway();
    }
    catch(const Ice::Exception& ex)
    {
        setPythonException(ex);
        return 0;
    }

    return createProxy(newProxy.value(), *self->communicator, reinterpret_cast<PyObject*>(Py_TYPE(self)));
}

#ifdef WIN32
extern "C"
#endif
static PyObject*
proxyIceIsBatchOneway(ProxyObject* self, PyObject* /*args*/)
{
    assert(self->proxy);

    PyObject* b;
    try
    {
        b = (*self->proxy)->ice_isBatchOneway() ? getTrue() : getFalse();
    }
    catch(const Ice::Exception& ex)
    {
        setPythonException(ex);
        return 0;
    }

    Py_INCREF(b);
    return b;
}

#ifdef WIN32
extern "C"
#endif
static PyObject*
proxyIceDatagram(ProxyObject* self, PyObject* /*args*/)
{
    assert(self->proxy);

    optional<Ice::ObjectPrx> newProxy;
    try
    {
        newProxy = (*self->proxy)->ice_datagram();
    }
    catch(const Ice::Exception& ex)
    {
        setPythonException(ex);
        return 0;
    }

    return createProxy(newProxy.value(), *self->communicator, reinterpret_cast<PyObject*>(Py_TYPE(self)));
}

#ifdef WIN32
extern "C"
#endif
static PyObject*
proxyIceIsDatagram(ProxyObject* self, PyObject* /*args*/)
{
    assert(self->proxy);

    PyObject* b;
    try
    {
        b = (*self->proxy)->ice_isDatagram() ? getTrue() : getFalse();
    }
    catch(const Ice::Exception& ex)
    {
        setPythonException(ex);
        return 0;
    }

    Py_INCREF(b);
    return b;
}

#ifdef WIN32
extern "C"
#endif
static PyObject*
proxyIceBatchDatagram(ProxyObject* self, PyObject* /*args*/)
{
    assert(self->proxy);

    optional<Ice::ObjectPrx> newProxy;
    try
    {
        newProxy = (*self->proxy)->ice_batchDatagram();
    }
    catch(const Ice::Exception& ex)
    {
        setPythonException(ex);
        return 0;
    }

    return createProxy(newProxy.value(), *self->communicator, reinterpret_cast<PyObject*>(Py_TYPE(self)));
}

#ifdef WIN32
extern "C"
#endif
static PyObject*
proxyIceIsBatchDatagram(ProxyObject* self, PyObject* /*args*/)
{
    assert(self->proxy);

    PyObject* b;
    try
    {
        b = (*self->proxy)->ice_isBatchDatagram() ? getTrue() : getFalse();
    }
    catch(const Ice::Exception& ex)
    {
        setPythonException(ex);
        return 0;
    }

    Py_INCREF(b);
    return b;
}

#ifdef WIN32
extern "C"
#endif
static PyObject*
proxyIceCompress(ProxyObject* self, PyObject* args)
{
    PyObject* flag;
    if(!PyArg_ParseTuple(args, STRCAST("O"), &flag))
    {
        return 0;
    }

    int n = PyObject_IsTrue(flag);
    if(n < 0)
    {
        return 0;
    }

    assert(self->proxy);

    optional<Ice::ObjectPrx> newProxy;
    try
    {
        newProxy = (*self->proxy)->ice_compress(n == 1);
    }
    catch(const Ice::Exception& ex)
    {
        setPythonException(ex);
        return 0;
    }

    return createProxy(newProxy.value(), *self->communicator, reinterpret_cast<PyObject*>(Py_TYPE(self)));
}

#ifdef WIN32
extern "C"
#endif
static PyObject*
proxyIceGetCompress(ProxyObject* self, PyObject* /*args*/)
{
    assert(self->proxy);

    PyObject* b;
    try
    {
        optional<bool> compress = (*self->proxy)->ice_getCompress();
        if(compress)
        {
            b = *compress ? getTrue() : getFalse();
        }
        else
        {
            b = Unset;
        }
    }
    catch(const Ice::Exception& ex)
    {
        setPythonException(ex);
        return 0;
    }
    Py_INCREF(b);
    return b;
}

#ifdef WIN32
extern "C"
#endif
static PyObject*
proxyIceTimeout(ProxyObject* self, PyObject* args)
{
    int timeout;
    if(!PyArg_ParseTuple(args, STRCAST("i"), &timeout))
    {
        return 0;
    }

    assert(self->proxy);

    optional<Ice::ObjectPrx> newProxy;
    try
    {
        newProxy = (*self->proxy)->ice_timeout(timeout);
    }
    catch(const invalid_argument& ex)
    {
        PyErr_Format(PyExc_RuntimeError, "%s", ex.what());
        return 0;
    }
    catch(const Ice::Exception& ex)
    {
        setPythonException(ex);
        return 0;
    }

    return createProxy(newProxy.value(), *self->communicator, reinterpret_cast<PyObject*>(Py_TYPE(self)));
}

#ifdef WIN32
extern "C"
#endif
static PyObject*
proxyIceGetTimeout(ProxyObject* self, PyObject* /*args*/)
{
    assert(self->proxy);

    try
    {
        optional<int> timeout = (*self->proxy)->ice_getTimeout();
        if(timeout)
        {
            return PyLong_FromLong(*timeout);
        }
        else
        {
            Py_INCREF(Unset);
            return Unset;
        }
    }
    catch(const Ice::Exception& ex)
    {
        setPythonException(ex);
        return 0;
    }
}

#ifdef WIN32
extern "C"
#endif
static PyObject*
proxyIceIsCollocationOptimized(ProxyObject* self, PyObject* /*args*/)
{
    assert(self->proxy);

    PyObject* b;
    try
    {
        b = (*self->proxy)->ice_isCollocationOptimized() ? getTrue() : getFalse();
    }
    catch(const Ice::Exception& ex)
    {
        setPythonException(ex);
        return 0;
    }

    Py_INCREF(b);
    return b;
}

#ifdef WIN32
extern "C"
#endif
static PyObject*
proxyIceCollocationOptimized(ProxyObject* self, PyObject* args)
{
    PyObject* flag;
    if(!PyArg_ParseTuple(args, STRCAST("O"), &flag))
    {
        return 0;
    }

    int n = PyObject_IsTrue(flag);
    if(n < 0)
    {
        return 0;
    }

    assert(self->proxy);

    optional<Ice::ObjectPrx> newProxy;
    try
    {
        newProxy = (*self->proxy)->ice_collocationOptimized(n == 1);
    }
    catch(const Ice::Exception& ex)
    {
        setPythonException(ex);
        return 0;
    }

    return createProxy(newProxy.value(), *self->communicator, reinterpret_cast<PyObject*>(Py_TYPE(self)));
}

#ifdef WIN32
extern "C"
#endif
static PyObject*
proxyIceConnectionId(ProxyObject* self, PyObject* args)
{
    PyObject* idObj;
    if(!PyArg_ParseTuple(args, STRCAST("O"), &idObj))
    {
        return 0;
    }

    string id;
    if(!getStringArg(idObj, "id", id))
    {
        return 0;
    }

    assert(self->proxy);

    optional<Ice::ObjectPrx> newProxy;
    try
    {
        newProxy = (*self->proxy)->ice_connectionId(id);
    }
    catch(const Ice::Exception& ex)
    {
        setPythonException(ex);
        return 0;
    }

    return createProxy(newProxy.value(), *self->communicator, reinterpret_cast<PyObject*>(Py_TYPE(self)));
}

#ifdef WIN32
extern "C"
#endif
static PyObject*
proxyIceFixed(ProxyObject* self, PyObject* args)
{
    assert(self->proxy);

    PyObject* p;
    if(!PyArg_ParseTuple(args, STRCAST("O"), &p))
    {
        return 0;
    }

    Ice::ConnectionPtr connection;
    if(!getConnectionArg(p, "ice_fixed", "connection", connection))
    {
        return 0;
    }

    optional<Ice::ObjectPrx> newProxy;
    try
    {
        newProxy = (*self->proxy)->ice_fixed(connection);
    }
    catch(const invalid_argument& ex)
    {
        PyErr_Format(PyExc_RuntimeError, "%s", ex.what());
        return 0;
    }
    catch(const Ice::Exception& ex)
    {
        setPythonException(ex);
        return 0;
    }

    return createProxy(newProxy.value(), *self->communicator, reinterpret_cast<PyObject*>(Py_TYPE(self)));
}

#ifdef WIN32
extern "C"
#endif
static PyObject*
proxyIceIsFixed(ProxyObject* self, PyObject* /*args*/)
{
    assert(self->proxy);

    PyObject* b;
    try
    {
        b = (*self->proxy)->ice_isFixed() ? getTrue() : getFalse();
    }
    catch(const Ice::Exception& ex)
    {
        setPythonException(ex);
        return 0;
    }

    Py_INCREF(b);
    return b;
}

#ifdef WIN32
extern "C"
#endif
static PyObject*
proxyIceGetConnection(ProxyObject* self, PyObject* /*args*/)
{
    assert(self->proxy);

    Ice::ConnectionPtr con;
    try
    {
        con = (*self->proxy)->ice_getConnection();
    }
    catch(const Ice::Exception& ex)
    {
        setPythonException(ex);
        return 0;
    }

    if(con)
    {
        return createConnection(con, *self->communicator);
    }
    else
    {
        Py_INCREF(Py_None);
        return Py_None;
    }
}

#ifdef WIN32
extern "C"
#endif
static PyObject*
proxyIceGetConnectionAsync(ProxyObject* self, PyObject* /*args*/, PyObject* /*kwds*/)
{
    assert(self->proxy);
    const string op = "ice_getConnection";

    auto callback = make_shared<GetConnectionAsyncCallback>(*self->communicator, op);
    function<void()> cancel;
    try
    {
        cancel = (*self->proxy)->ice_getConnectionAsync(
            [callback](const Ice::ConnectionPtr& connection)
            {
                callback->response(connection);
            },
            [callback](exception_ptr exptr)
            {
                try
                {
                    rethrow_exception(exptr);
                }
                catch (const Ice::Exception& ex)
                {
                    callback->exception(ex);
                }
            });
    }
    catch (const Ice::Exception& ex)
    {
        setPythonException(ex);
        return 0;
    }

    PyObjectHandle asyncInvocationContextObj = createAsyncInvocationContext(cancel, *self->communicator);
    if (!asyncInvocationContextObj.get())
    {
        return 0;
    }

    PyObjectHandle future = createFuture(op, asyncInvocationContextObj.get());
    if (!future.get())
    {
        return 0;
    }
    callback->setFuture(future.get());
    return future.release();
}

#ifdef WIN32
extern "C"
#endif
static PyObject*
proxyIceGetCachedConnection(ProxyObject* self, PyObject* /*args*/)
{
    assert(self->proxy);

    Ice::ConnectionPtr con;
    try
    {
        con = (*self->proxy)->ice_getCachedConnection();
    }
    catch(const Ice::Exception& ex)
    {
        setPythonException(ex);
        return 0;
    }

    if(con)
    {
        return createConnection(con, *self->communicator);
    }
    else
    {
        Py_INCREF(Py_None);
        return Py_None;
    }
}

#ifdef WIN32
extern "C"
#endif
static PyObject*
proxyIceFlushBatchRequests(ProxyObject* self, PyObject* /*args*/)
{
    assert(self->proxy);

    try
    {
        AllowThreads allowThreads; // Release Python's global interpreter lock during remote invocations.
        (*self->proxy)->ice_flushBatchRequests();
    }
    catch(const Ice::Exception& ex)
    {
        setPythonException(ex);
        return 0;
    }

    Py_INCREF(Py_None);
    return Py_None;
}

#ifdef WIN32
extern "C"
#endif
static PyObject*
proxyIceFlushBatchRequestsAsync(ProxyObject* self, PyObject* /*args*/, PyObject* /*kwds*/)
{
    assert(self->proxy);
    const string op = "ice_flushBatchRequests";

    auto callback = make_shared<FlushAsyncCallback>(op);
    function<void()> cancel;
    try
    {
        cancel = (*self->proxy)->ice_flushBatchRequestsAsync(
            [callback](exception_ptr exptr)
            {
                try
                {
                    rethrow_exception(exptr);
                }
                catch (const Ice::Exception& ex)
                {
                    callback->exception(ex);
                }
                catch (...)
                {
                    assert(false);
                }
            },
            [callback](bool sentSynchronously)
            {
                callback->sent(sentSynchronously);
            });
    }
    catch(const Ice::Exception& ex)
    {
        setPythonException(ex);
        return 0;
    }

    PyObjectHandle asyncInvocationContextObj = createAsyncInvocationContext(std::move(cancel), *self->communicator);
    if(!asyncInvocationContextObj.get())
    {
        return 0;
    }

    PyObjectHandle future = createFuture(op, asyncInvocationContextObj.get());
    if(!future.get())
    {
        return 0;
    }
    callback->setFuture(future.get());
    return future.release();
}

#ifdef WIN32
extern "C"
#endif
static PyObject*
proxyIceInvoke(ProxyObject* self, PyObject* args)
{
    return iceInvoke(reinterpret_cast<PyObject*>(self), args);
}

#ifdef WIN32
extern "C"
#endif
static PyObject*
proxyIceInvokeAsync(ProxyObject* self, PyObject* args, PyObject* /*kwds*/)
{
    return iceInvokeAsync(reinterpret_cast<PyObject*>(self), args);
}

static PyObject*
checkedCastImpl(ProxyObject* p, const string& id, PyObject* facet, PyObject* ctx, PyObject* type)
{
    optional<Ice::ObjectPrx> target;
    if(!facet || facet == Py_None)
    {
        target = *p->proxy;
    }
    else
    {
        string facetStr = getString(facet);
        target = (*p->proxy)->ice_facet(facetStr);
    }
    assert(target);

    bool b = false;
    try
    {
        Ice::Context c;
        if(ctx && ctx != Py_None)
        {
            if(!dictionaryToContext(ctx, c))
            {
                return 0;
            }
        }

        AllowThreads allowThreads; // Release Python's global interpreter lock during remote invocations.
        b = target->ice_isA(id, (ctx == 0 || ctx == Py_None) ? Ice::noExplicitContext : c);
    }
    catch(const Ice::FacetNotExistException&)
    {
        // Ignore.
    }
    catch(const Ice::Exception& ex)
    {
        setPythonException(ex);
        return 0;
    }

    if(b)
    {
        return createProxy(target.value(), *p->communicator, type);
    }

    Py_INCREF(Py_None);
    return Py_None;
}

#ifdef WIN32
extern "C"
#endif
static PyObject*
proxyIceCheckedCast(PyObject* type, PyObject* args)
{
    //
    // ice_checkedCast is called from generated code, therefore we always expect
    // to receive four arguments.
    //
    PyObject* obj;
    char* id;
    PyObject* facetOrContext = 0;
    PyObject* ctx = 0;
    if(!PyArg_ParseTuple(args, STRCAST("OsOO"), &obj, &id, &facetOrContext, &ctx))
    {
        return 0;
    }

    if(obj == Py_None)
    {
        Py_INCREF(Py_None);
        return Py_None;
    }

    if(!checkProxy(obj))
    {
        PyErr_Format(PyExc_ValueError, STRCAST("ice_checkedCast requires a proxy argument"));
        return 0;
    }

    PyObject* facet = 0;

    if(checkString(facetOrContext))
    {
        facet = facetOrContext;
    }
    else if(PyDict_Check(facetOrContext))
    {
        if(ctx != Py_None)
        {
            PyErr_Format(PyExc_ValueError, STRCAST("facet argument to checkedCast must be a string"));
            return 0;
        }
        ctx = facetOrContext;
    }
    else if(facetOrContext != Py_None)
    {
        PyErr_Format(PyExc_ValueError, STRCAST("second argument to checkedCast must be a facet or context"));
        return 0;
    }

    if(ctx != Py_None && !PyDict_Check(ctx))
    {
        PyErr_Format(PyExc_ValueError, STRCAST("context argument to checkedCast must be a dictionary"));
        return 0;
    }

    return checkedCastImpl(reinterpret_cast<ProxyObject*>(obj), id, facet, ctx, type);
}

#ifdef WIN32
extern "C"
#endif
static PyObject*
proxyIceUncheckedCast(PyObject* type, PyObject* args)
{
    //
    // ice_uncheckedCast is called from generated code, therefore we always expect
    // to receive two arguments.
    //
    PyObject* obj;
    char* facet = 0;
    if(!PyArg_ParseTuple(args, STRCAST("Oz"), &obj, &facet))
    {
        return 0;
    }

    if(obj == Py_None)
    {
        Py_INCREF(Py_None);
        return Py_None;
    }

    if(!checkProxy(obj))
    {
        PyErr_Format(PyExc_ValueError, STRCAST("ice_uncheckedCast requires a proxy argument"));
        return 0;
    }

    ProxyObject* p = reinterpret_cast<ProxyObject*>(obj);

    if(facet)
    {
        return createProxy((*p->proxy)->ice_facet(facet), *p->communicator, type);
    }
    else
    {
        return createProxy(*p->proxy, *p->communicator, type);
    }
}

#ifdef WIN32
extern "C"
#endif
static PyObject*
proxyCheckedCast(PyObject* /*self*/, PyObject* args)
{
    PyObject* obj;
    PyObject* arg1 = 0;
    PyObject* arg2 = 0;
    if(!PyArg_ParseTuple(args, STRCAST("O|OO"), &obj, &arg1, &arg2))
    {
        return 0;
    }

    if(obj == Py_None)
    {
        Py_INCREF(Py_None);
        return Py_None;
    }

    if(!checkProxy(obj))
    {
        PyErr_Format(PyExc_ValueError, STRCAST("checkedCast requires a proxy argument"));
        return 0;
    }

    PyObject* facet = 0;
    PyObject* ctx = 0;

    if(arg1 != 0 && arg2 != 0)
    {
        if(arg1 == Py_None)
        {
            arg1 = 0;
        }

        if(arg2 == Py_None)
        {
            arg2 = 0;
        }

        if(arg1 != 0)
        {
            if(!checkString(arg1))
            {
                PyErr_Format(PyExc_ValueError, STRCAST("facet argument to checkedCast must be a string"));
                return 0;
            }
            facet = arg1;
        }

        if(arg2 != 0 && !PyDict_Check(arg2))
        {
            PyErr_Format(PyExc_ValueError, STRCAST("context argument to checkedCast must be a dictionary"));
            return 0;
        }
        ctx = arg2;
    }
    else if(arg1 != 0 && arg1 != Py_None)
    {
        if(checkString(arg1))
        {
            facet = arg1;
        }
        else if(PyDict_Check(arg1))
        {
            ctx = arg1;
        }
        else
        {
            PyErr_Format(PyExc_ValueError, STRCAST("second argument to checkedCast must be a facet or context"));
            return 0;
        }
    }

    return checkedCastImpl(reinterpret_cast<ProxyObject*>(obj), "::Ice::Object", facet, ctx, 0);
}

#ifdef WIN32
extern "C"
#endif
static PyObject*
proxyUncheckedCast(PyObject* /*self*/, PyObject* args)
{
    PyObject* obj;
    PyObject* facetObj = 0;
    if(!PyArg_ParseTuple(args, STRCAST("O|O"), &obj, &facetObj))
    {
        return 0;
    }

    if(obj == Py_None)
    {
        Py_INCREF(Py_None);
        return Py_None;
    }

    string facet;
    if(facetObj)
    {
        if(!getStringArg(facetObj, "facet", facet))
        {
            return 0;
        }
    }

    if(!checkProxy(obj))
    {
        PyErr_Format(PyExc_ValueError, STRCAST("uncheckedCast requires a proxy argument"));
        return 0;
    }

    ProxyObject* p = reinterpret_cast<ProxyObject*>(obj);

    if(facetObj)
    {
        return createProxy((*p->proxy)->ice_facet(facet), *p->communicator);
    }
    else
    {
        return createProxy(*p->proxy, *p->communicator);
    }
}

#ifdef WIN32
extern "C"
#endif
static PyObject*
proxyIceStaticId(PyObject* /*self*/, PyObject* /*args*/)
{
    return createString(Ice::Object::ice_staticId());
}

static PyMethodDef ProxyMethods[] =
{
    { STRCAST("ice_getCommunicator"), reinterpret_cast<PyCFunction>(proxyIceGetCommunicator), METH_NOARGS,
        PyDoc_STR(STRCAST("ice_getCommunicator() -> Ice.Communicator")) },
    { STRCAST("ice_toString"), reinterpret_cast<PyCFunction>(proxyRepr), METH_NOARGS,
        PyDoc_STR(STRCAST("ice_toString() -> string")) },
    { STRCAST("ice_isA"), reinterpret_cast<PyCFunction>(proxyIceIsA), METH_VARARGS,
        PyDoc_STR(STRCAST("ice_isA(type, [ctx]) -> bool")) },
    { STRCAST("ice_isAAsync"), reinterpret_cast<PyCFunction>(proxyIceIsAAsync), METH_VARARGS,
        PyDoc_STR(STRCAST("ice_isAAsync(type, [ctx]) -> Ice.Future")) },
    { STRCAST("ice_ping"), reinterpret_cast<PyCFunction>(proxyIcePing), METH_VARARGS,
        PyDoc_STR(STRCAST("ice_ping([ctx]) -> None")) },
    { STRCAST("ice_pingAsync"), reinterpret_cast<PyCFunction>(proxyIcePingAsync), METH_VARARGS,
        PyDoc_STR(STRCAST("ice_pingAsync([ctx]) -> Ice.Future")) },
    { STRCAST("ice_ids"), reinterpret_cast<PyCFunction>(proxyIceIds), METH_VARARGS,
        PyDoc_STR(STRCAST("ice_ids([ctx]) -> list")) },
    { STRCAST("ice_idsAsync"), reinterpret_cast<PyCFunction>(proxyIceIdsAsync), METH_VARARGS,
        PyDoc_STR(STRCAST("ice_idsAsync([ctx]) -> Ice.Future")) },
    { STRCAST("ice_id"), reinterpret_cast<PyCFunction>(proxyIceId), METH_VARARGS,
        PyDoc_STR(STRCAST("ice_id([ctx]) -> string")) },
    { STRCAST("ice_idAsync"), reinterpret_cast<PyCFunction>(proxyIceIdAsync), METH_VARARGS,
        PyDoc_STR(STRCAST("ice_idAsync([ctx]) -> Ice.Future")) },
    { STRCAST("ice_getIdentity"), reinterpret_cast<PyCFunction>(proxyIceGetIdentity), METH_NOARGS,
        PyDoc_STR(STRCAST("ice_getIdentity() -> Ice.Identity")) },
    { STRCAST("ice_identity"), reinterpret_cast<PyCFunction>(proxyIceIdentity), METH_VARARGS,
        PyDoc_STR(STRCAST("ice_identity(id) -> Ice.ObjectPrx")) },
    { STRCAST("ice_getContext"), reinterpret_cast<PyCFunction>(proxyIceGetContext), METH_NOARGS,
        PyDoc_STR(STRCAST("ice_getContext() -> dict")) },
    { STRCAST("ice_context"), reinterpret_cast<PyCFunction>(proxyIceContext), METH_VARARGS,
        PyDoc_STR(STRCAST("ice_context(dict) -> Ice.ObjectPrx")) },
    { STRCAST("ice_getFacet"), reinterpret_cast<PyCFunction>(proxyIceGetFacet), METH_NOARGS,
        PyDoc_STR(STRCAST("ice_getFacet() -> string")) },
    { STRCAST("ice_facet"), reinterpret_cast<PyCFunction>(proxyIceFacet), METH_VARARGS,
        PyDoc_STR(STRCAST("ice_facet(string) -> Ice.ObjectPrx")) },
    { STRCAST("ice_getAdapterId"), reinterpret_cast<PyCFunction>(proxyIceGetAdapterId), METH_NOARGS,
        PyDoc_STR(STRCAST("ice_getAdapterId() -> string")) },
    { STRCAST("ice_adapterId"), reinterpret_cast<PyCFunction>(proxyIceAdapterId), METH_VARARGS,
        PyDoc_STR(STRCAST("ice_adapterId(string) -> proxy")) },
    { STRCAST("ice_getEndpoints"), reinterpret_cast<PyCFunction>(proxyIceGetEndpoints), METH_NOARGS,
        PyDoc_STR(STRCAST("ice_getEndpoints() -> tuple")) },
    { STRCAST("ice_endpoints"), reinterpret_cast<PyCFunction>(proxyIceEndpoints), METH_VARARGS,
        PyDoc_STR(STRCAST("ice_endpoints(tuple) -> proxy")) },
    { STRCAST("ice_getLocatorCacheTimeout"), reinterpret_cast<PyCFunction>(proxyIceGetLocatorCacheTimeout),
        METH_NOARGS, PyDoc_STR(STRCAST("ice_getLocatorCacheTimeout() -> int")) },
    { STRCAST("ice_getInvocationTimeout"), reinterpret_cast<PyCFunction>(proxyIceGetInvocationTimeout),
        METH_NOARGS, PyDoc_STR(STRCAST("ice_getInvocationTimeout() -> int")) },
    { STRCAST("ice_getConnectionId"), reinterpret_cast<PyCFunction>(proxyIceGetConnectionId),
        METH_NOARGS, PyDoc_STR(STRCAST("ice_getConnectionId() -> string")) },
    { STRCAST("ice_isCollocationOptimized"), reinterpret_cast<PyCFunction>(proxyIceIsCollocationOptimized), METH_NOARGS,
        PyDoc_STR(STRCAST("ice_isCollocationOptimized() -> bool")) },
    { STRCAST("ice_collocationOptimized"), reinterpret_cast<PyCFunction>(proxyIceCollocationOptimized), METH_VARARGS,
        PyDoc_STR(STRCAST("ice_collocationOptimized(bool) -> Ice.ObjectPrx")) },
    { STRCAST("ice_locatorCacheTimeout"), reinterpret_cast<PyCFunction>(proxyIceLocatorCacheTimeout), METH_VARARGS,
        PyDoc_STR(STRCAST("ice_locatorCacheTimeout(int) -> Ice.ObjectPrx")) },
    { STRCAST("ice_invocationTimeout"), reinterpret_cast<PyCFunction>(proxyIceInvocationTimeout), METH_VARARGS,
        PyDoc_STR(STRCAST("ice_invocationTimeout(int) -> Ice.ObjectPrx")) },
    { STRCAST("ice_isConnectionCached"), reinterpret_cast<PyCFunction>(proxyIceIsConnectionCached), METH_NOARGS,
        PyDoc_STR(STRCAST("ice_isConnectionCached() -> bool")) },
    { STRCAST("ice_connectionCached"), reinterpret_cast<PyCFunction>(proxyIceConnectionCached), METH_VARARGS,
        PyDoc_STR(STRCAST("ice_connectionCached(bool) -> Ice.ObjectPrx")) },
    { STRCAST("ice_getEndpointSelection"), reinterpret_cast<PyCFunction>(proxyIceGetEndpointSelection), METH_NOARGS,
        PyDoc_STR(STRCAST("ice_getEndpointSelection() -> bool")) },
    { STRCAST("ice_endpointSelection"), reinterpret_cast<PyCFunction>(proxyIceEndpointSelection), METH_VARARGS,
        PyDoc_STR(STRCAST("ice_endpointSelection(Ice.EndpointSelectionType) -> Ice.ObjectPrx")) },
    { STRCAST("ice_isSecure"), reinterpret_cast<PyCFunction>(proxyIceIsSecure), METH_NOARGS,
        PyDoc_STR(STRCAST("ice_isSecure() -> bool")) },
    { STRCAST("ice_secure"), reinterpret_cast<PyCFunction>(proxyIceSecure), METH_VARARGS,
        PyDoc_STR(STRCAST("ice_secure(bool) -> Ice.ObjectPrx")) },
    { STRCAST("ice_getEncodingVersion"), reinterpret_cast<PyCFunction>(proxyIceGetEncodingVersion), METH_NOARGS,
        PyDoc_STR(STRCAST("ice_getEncodingVersion() -> Ice.EncodingVersion")) },
    { STRCAST("ice_encodingVersion"), reinterpret_cast<PyCFunction>(proxyIceEncodingVersion), METH_VARARGS,
        PyDoc_STR(STRCAST("ice_endpointSelection(Ice.EncodingVersion) -> Ice.ObjectPrx")) },
    { STRCAST("ice_isPreferSecure"), reinterpret_cast<PyCFunction>(proxyIceIsPreferSecure), METH_NOARGS,
        PyDoc_STR(STRCAST("ice_isPreferSecure() -> bool")) },
    { STRCAST("ice_preferSecure"), reinterpret_cast<PyCFunction>(proxyIcePreferSecure), METH_VARARGS,
        PyDoc_STR(STRCAST("ice_preferSecure(bool) -> Ice.ObjectPrx")) },
    { STRCAST("ice_getRouter"), reinterpret_cast<PyCFunction>(proxyIceGetRouter), METH_NOARGS,
        PyDoc_STR(STRCAST("ice_getRouter() -> Ice.RouterPrx")) },
    { STRCAST("ice_router"), reinterpret_cast<PyCFunction>(proxyIceRouter), METH_VARARGS,
        PyDoc_STR(STRCAST("ice_router(Ice.RouterPrx) -> Ice.ObjectPrx")) },
    { STRCAST("ice_getLocator"), reinterpret_cast<PyCFunction>(proxyIceGetLocator), METH_NOARGS,
        PyDoc_STR(STRCAST("ice_getLocator() -> Ice.LocatorPrx")) },
    { STRCAST("ice_locator"), reinterpret_cast<PyCFunction>(proxyIceLocator), METH_VARARGS,
        PyDoc_STR(STRCAST("ice_locator(Ice.LocatorPrx) -> Ice.ObjectPrx")) },
    { STRCAST("ice_twoway"), reinterpret_cast<PyCFunction>(proxyIceTwoway), METH_NOARGS,
        PyDoc_STR(STRCAST("ice_twoway() -> Ice.ObjectPrx")) },
    { STRCAST("ice_isTwoway"), reinterpret_cast<PyCFunction>(proxyIceIsTwoway), METH_NOARGS,
        PyDoc_STR(STRCAST("ice_isTwoway() -> bool")) },
    { STRCAST("ice_oneway"), reinterpret_cast<PyCFunction>(proxyIceOneway), METH_NOARGS,
        PyDoc_STR(STRCAST("ice_oneway() -> Ice.ObjectPrx")) },
    { STRCAST("ice_isOneway"), reinterpret_cast<PyCFunction>(proxyIceIsOneway), METH_NOARGS,
        PyDoc_STR(STRCAST("ice_isOneway() -> bool")) },
    { STRCAST("ice_batchOneway"), reinterpret_cast<PyCFunction>(proxyIceBatchOneway), METH_NOARGS,
        PyDoc_STR(STRCAST("ice_batchOneway() -> Ice.ObjectPrx")) },
    { STRCAST("ice_isBatchOneway"), reinterpret_cast<PyCFunction>(proxyIceIsBatchOneway), METH_NOARGS,
        PyDoc_STR(STRCAST("ice_isBatchOneway() -> bool")) },
    { STRCAST("ice_datagram"), reinterpret_cast<PyCFunction>(proxyIceDatagram), METH_NOARGS,
        PyDoc_STR(STRCAST("ice_datagram() -> Ice.ObjectPrx")) },
    { STRCAST("ice_isDatagram"), reinterpret_cast<PyCFunction>(proxyIceIsDatagram), METH_NOARGS,
        PyDoc_STR(STRCAST("ice_isDatagram() -> bool")) },
    { STRCAST("ice_batchDatagram"), reinterpret_cast<PyCFunction>(proxyIceBatchDatagram), METH_NOARGS,
        PyDoc_STR(STRCAST("ice_batchDatagram() -> Ice.ObjectPrx")) },
    { STRCAST("ice_isBatchDatagram"), reinterpret_cast<PyCFunction>(proxyIceIsBatchDatagram), METH_NOARGS,
        PyDoc_STR(STRCAST("ice_isBatchDatagram() -> bool")) },
    { STRCAST("ice_compress"), reinterpret_cast<PyCFunction>(proxyIceCompress), METH_VARARGS,
        PyDoc_STR(STRCAST("ice_compress(bool) -> Ice.ObjectPrx")) },
    { STRCAST("ice_getCompress"), reinterpret_cast<PyCFunction>(proxyIceGetCompress), METH_VARARGS,
        PyDoc_STR(STRCAST("ice_getCompress() -> bool")) },
    { STRCAST("ice_timeout"), reinterpret_cast<PyCFunction>(proxyIceTimeout), METH_VARARGS,
        PyDoc_STR(STRCAST("ice_timeout(int) -> Ice.ObjectPrx")) },
    { STRCAST("ice_getTimeout"), reinterpret_cast<PyCFunction>(proxyIceGetTimeout), METH_VARARGS,
        PyDoc_STR(STRCAST("ice_getTimeout() -> int")) },
    { STRCAST("ice_connectionId"), reinterpret_cast<PyCFunction>(proxyIceConnectionId), METH_VARARGS,
        PyDoc_STR(STRCAST("ice_connectionId(string) -> Ice.ObjectPrx")) },
    { STRCAST("ice_fixed"), reinterpret_cast<PyCFunction>(proxyIceFixed), METH_VARARGS,
        PyDoc_STR(STRCAST("ice_fixed(Ice.Connection) -> Ice.ObjectPrx")) },
    { STRCAST("ice_isFixed"), reinterpret_cast<PyCFunction>(proxyIceIsFixed), METH_NOARGS,
        PyDoc_STR(STRCAST("ice_isFixed() -> bool")) },
    { STRCAST("ice_getConnection"), reinterpret_cast<PyCFunction>(proxyIceGetConnection), METH_NOARGS,
        PyDoc_STR(STRCAST("ice_getConnection() -> Ice.Connection")) },
    { STRCAST("ice_getConnectionAsync"), reinterpret_cast<PyCFunction>(proxyIceGetConnectionAsync),
        METH_NOARGS, PyDoc_STR(STRCAST("ice_getConnectionAsync() -> Ice.Future")) },
    { STRCAST("ice_getCachedConnection"), reinterpret_cast<PyCFunction>(proxyIceGetCachedConnection), METH_NOARGS,
        PyDoc_STR(STRCAST("ice_getCachedConnection() -> Ice.Connection")) },
    { STRCAST("ice_flushBatchRequests"), reinterpret_cast<PyCFunction>(proxyIceFlushBatchRequests), METH_NOARGS,
        PyDoc_STR(STRCAST("ice_flushBatchRequests() -> void")) },
    { STRCAST("ice_flushBatchRequestsAsync"), reinterpret_cast<PyCFunction>(proxyIceFlushBatchRequestsAsync),
        METH_NOARGS, PyDoc_STR(STRCAST("ice_flushBatchRequestsAsync() -> Ice.Future")) },
    { STRCAST("ice_invoke"), reinterpret_cast<PyCFunction>(proxyIceInvoke), METH_VARARGS,
        PyDoc_STR(STRCAST("ice_invoke(operation, mode, inParams) -> bool, outParams")) },
    { STRCAST("ice_invokeAsync"), reinterpret_cast<PyCFunction>(proxyIceInvokeAsync), METH_VARARGS | METH_KEYWORDS,
        PyDoc_STR(STRCAST("ice_invokeAsync(op, mode, inParams[, context]) -> Ice.Future")) },
    { STRCAST("ice_checkedCast"), reinterpret_cast<PyCFunction>(proxyIceCheckedCast), METH_VARARGS | METH_CLASS,
        PyDoc_STR(STRCAST("ice_checkedCast(proxy, id[, facetOrContext[, context]]) -> proxy")) },
    { STRCAST("ice_uncheckedCast"), reinterpret_cast<PyCFunction>(proxyIceUncheckedCast), METH_VARARGS | METH_CLASS,
        PyDoc_STR(STRCAST("ice_uncheckedCast(proxy) -> proxy")) },
    { STRCAST("checkedCast"), reinterpret_cast<PyCFunction>(proxyCheckedCast), METH_VARARGS | METH_STATIC,
        PyDoc_STR(STRCAST("checkedCast(proxy) -> proxy")) },
    { STRCAST("uncheckedCast"), reinterpret_cast<PyCFunction>(proxyUncheckedCast), METH_VARARGS | METH_STATIC,
        PyDoc_STR(STRCAST("uncheckedCast(proxy) -> proxy")) },
    { STRCAST("ice_staticId"), reinterpret_cast<PyCFunction>(proxyIceStaticId), METH_NOARGS | METH_STATIC,
        PyDoc_STR(STRCAST("ice_staticId() -> string")) },
    { 0, 0 } /* sentinel */
};

namespace IcePy
{

PyTypeObject ProxyType =
{
    /* The ob_type field must be initialized in the module init function
     * to be portable to Windows without using C++. */
    PyVarObject_HEAD_INIT(0, 0)
    STRCAST("IcePy.ObjectPrx"),      /* tp_name */
    sizeof(ProxyObject),             /* tp_basicsize */
    0,                               /* tp_itemsize */
    /* methods */
    reinterpret_cast<destructor>(proxyDealloc), /* tp_dealloc */
    0,                               /* tp_print */
    0,                               /* tp_getattr */
    0,                               /* tp_setattr */
    0,                               /* tp_reserved */
    reinterpret_cast<reprfunc>(proxyRepr), /* tp_repr */
    0,                               /* tp_as_number */
    0,                               /* tp_as_sequence */
    0,                               /* tp_as_mapping */
    reinterpret_cast<hashfunc>(proxyHash), /* tp_hash */
    0,                               /* tp_call */
    0,                               /* tp_str */
    0,                               /* tp_getattro */
    0,                               /* tp_setattro */
    0,                               /* tp_as_buffer */
    Py_TPFLAGS_BASETYPE,             /* tp_flags */
    0,                               /* tp_doc */
    0,                               /* tp_traverse */
    0,                               /* tp_clear */
    reinterpret_cast<richcmpfunc>(proxyCompare), /* tp_richcompare */
    0,                               /* tp_weaklistoffset */
    0,                               /* tp_iter */
    0,                               /* tp_iternext */
    ProxyMethods,                    /* tp_methods */
    0,                               /* tp_members */
    0,                               /* tp_getset */
    0,                               /* tp_base */
    0,                               /* tp_dict */
    0,                               /* tp_descr_get */
    0,                               /* tp_descr_set */
    0,                               /* tp_dictoffset */
    0,                               /* tp_init */
    0,                               /* tp_alloc */
    reinterpret_cast<newfunc>(proxyNew), /* tp_new */
    0,                               /* tp_free */
    0,                               /* tp_is_gc */
};

}

bool
IcePy::initProxy(PyObject* module)
{
    if(PyType_Ready(&ProxyType) < 0)
    {
        return false;
    }
    PyTypeObject* proxyType = &ProxyType; // Necessary to prevent GCC's strict-alias warnings.
    if(PyModule_AddObject(module, STRCAST("ObjectPrx"), reinterpret_cast<PyObject*>(proxyType)) < 0)
    {
        return false;
    }

    return true;
}

PyObject*
IcePy::createProxy(const Ice::ObjectPrx& proxy, const Ice::CommunicatorPtr& communicator, PyObject* type)
{
    if(!type)
    {
        PyTypeObject* proxyType = &ProxyType; // Necessary to prevent GCC's strict-alias warnings.
        type = reinterpret_cast<PyObject*>(proxyType);
    }
    return reinterpret_cast<PyObject*>(allocateProxy(proxy, communicator, type));
}

bool
IcePy::checkProxy(PyObject* p)
{
    PyTypeObject* type = &ProxyType; // Necessary to prevent GCC's strict-alias warnings.
    return PyObject_IsInstance(p, reinterpret_cast<PyObject*>(type)) == 1;
}

Ice::ObjectPrx
IcePy::getProxy(PyObject* p)
{
    assert(checkProxy(p));
    ProxyObject* obj = reinterpret_cast<ProxyObject*>(p);
    return *obj->proxy;
}

bool
IcePy::getProxyArg(PyObject* p, const string& func, const string& arg, optional<Ice::ObjectPrx>& proxy, const string& type)
{
    bool result = true;

    if(checkProxy(p))
    {
        if(!type.empty())
        {
            PyObject* proxyType = lookupType(type);
            assert(proxyType);
            if(!PyObject_IsInstance(p, proxyType))
            {
                result = false;
            }
        }
    }
    else if(p != Py_None)
    {
        result = false;
    }

    if(result)
    {
        if(p != Py_None)
        {
            ProxyObject* obj = reinterpret_cast<ProxyObject*>(p);
            proxy = *obj->proxy;
        }
        else
        {
            proxy = nullopt;
        }
    }
    else
    {
        string typeName = type.empty() ? "Ice.ObjectPrx" : type;
        PyErr_Format(PyExc_ValueError, STRCAST("%s expects a proxy of type %s or None for argument '%s'"),
                     func.c_str(), typeName.c_str(), arg.c_str());
    }

    return result;
}

Ice::CommunicatorPtr
IcePy::getProxyCommunicator(PyObject* p)
{
    assert(checkProxy(p));
    ProxyObject* obj = reinterpret_cast<ProxyObject*>(p);
    return *obj->communicator;
}
