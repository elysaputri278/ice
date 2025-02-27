# encoding: utf-8
#
# Copyright (c) ZeroC, Inc. All rights reserved.
#
#
# Ice version 3.7.10
#
# <auto-generated>
#
# Generated from file `LocalException.ice'
#
# Warning: do not edit this file.
#
# </auto-generated>
#

require 'Ice'
require 'Ice/Identity.rb'
require 'Ice/Version.rb'
require 'Ice/BuiltinSequences.rb'

module ::Ice

    if not defined?(::Ice::InitializationException)
        class InitializationException < Ice::LocalException
            def initialize(reason='')
                @reason = reason
            end

            def to_s
                '::Ice::InitializationException'
            end

            attr_accessor :reason
        end

        T_InitializationException = ::Ice::__defineException('::Ice::InitializationException', InitializationException, false, nil, [["reason", ::Ice::T_string, false, 0]])
    end

    if not defined?(::Ice::PluginInitializationException)
        class PluginInitializationException < Ice::LocalException
            def initialize(reason='')
                @reason = reason
            end

            def to_s
                '::Ice::PluginInitializationException'
            end

            attr_accessor :reason
        end

        T_PluginInitializationException = ::Ice::__defineException('::Ice::PluginInitializationException', PluginInitializationException, false, nil, [["reason", ::Ice::T_string, false, 0]])
    end

    if not defined?(::Ice::CollocationOptimizationException)
        class CollocationOptimizationException < Ice::LocalException
            def initialize
            end

            def to_s
                '::Ice::CollocationOptimizationException'
            end
        end

        T_CollocationOptimizationException = ::Ice::__defineException('::Ice::CollocationOptimizationException', CollocationOptimizationException, false, nil, [])
    end

    if not defined?(::Ice::AlreadyRegisteredException)
        class AlreadyRegisteredException < Ice::LocalException
            def initialize(kindOfObject='', id='')
                @kindOfObject = kindOfObject
                @id = id
            end

            def to_s
                '::Ice::AlreadyRegisteredException'
            end

            attr_accessor :kindOfObject, :id
        end

        T_AlreadyRegisteredException = ::Ice::__defineException('::Ice::AlreadyRegisteredException', AlreadyRegisteredException, false, nil, [
            ["kindOfObject", ::Ice::T_string, false, 0],
            ["id", ::Ice::T_string, false, 0]
        ])
    end

    if not defined?(::Ice::NotRegisteredException)
        class NotRegisteredException < Ice::LocalException
            def initialize(kindOfObject='', id='')
                @kindOfObject = kindOfObject
                @id = id
            end

            def to_s
                '::Ice::NotRegisteredException'
            end

            attr_accessor :kindOfObject, :id
        end

        T_NotRegisteredException = ::Ice::__defineException('::Ice::NotRegisteredException', NotRegisteredException, false, nil, [
            ["kindOfObject", ::Ice::T_string, false, 0],
            ["id", ::Ice::T_string, false, 0]
        ])
    end

    if not defined?(::Ice::TwowayOnlyException)
        class TwowayOnlyException < Ice::LocalException
            def initialize(operation='')
                @operation = operation
            end

            def to_s
                '::Ice::TwowayOnlyException'
            end

            attr_accessor :operation
        end

        T_TwowayOnlyException = ::Ice::__defineException('::Ice::TwowayOnlyException', TwowayOnlyException, false, nil, [["operation", ::Ice::T_string, false, 0]])
    end

    if not defined?(::Ice::CloneNotImplementedException)
        class CloneNotImplementedException < Ice::LocalException
            def initialize
            end

            def to_s
                '::Ice::CloneNotImplementedException'
            end
        end

        T_CloneNotImplementedException = ::Ice::__defineException('::Ice::CloneNotImplementedException', CloneNotImplementedException, false, nil, [])
    end

    if not defined?(::Ice::UnknownException)
        class UnknownException < Ice::LocalException
            def initialize(unknown='')
                @unknown = unknown
            end

            def to_s
                '::Ice::UnknownException'
            end

            attr_accessor :unknown
        end

        T_UnknownException = ::Ice::__defineException('::Ice::UnknownException', UnknownException, false, nil, [["unknown", ::Ice::T_string, false, 0]])
    end

    if not defined?(::Ice::UnknownLocalException)
        class UnknownLocalException < ::Ice::UnknownException
            def initialize(unknown='')
                super(unknown)
            end

            def to_s
                '::Ice::UnknownLocalException'
            end
        end

        T_UnknownLocalException = ::Ice::__defineException('::Ice::UnknownLocalException', UnknownLocalException, false, ::Ice::T_UnknownException, [])
    end

    if not defined?(::Ice::UnknownUserException)
        class UnknownUserException < ::Ice::UnknownException
            def initialize(unknown='')
                super(unknown)
            end

            def to_s
                '::Ice::UnknownUserException'
            end
        end

        T_UnknownUserException = ::Ice::__defineException('::Ice::UnknownUserException', UnknownUserException, false, ::Ice::T_UnknownException, [])
    end

    if not defined?(::Ice::VersionMismatchException)
        class VersionMismatchException < Ice::LocalException
            def initialize
            end

            def to_s
                '::Ice::VersionMismatchException'
            end
        end

        T_VersionMismatchException = ::Ice::__defineException('::Ice::VersionMismatchException', VersionMismatchException, false, nil, [])
    end

    if not defined?(::Ice::CommunicatorDestroyedException)
        class CommunicatorDestroyedException < Ice::LocalException
            def initialize
            end

            def to_s
                '::Ice::CommunicatorDestroyedException'
            end
        end

        T_CommunicatorDestroyedException = ::Ice::__defineException('::Ice::CommunicatorDestroyedException', CommunicatorDestroyedException, false, nil, [])
    end

    if not defined?(::Ice::ObjectAdapterDeactivatedException)
        class ObjectAdapterDeactivatedException < Ice::LocalException
            def initialize(name='')
                @name = name
            end

            def to_s
                '::Ice::ObjectAdapterDeactivatedException'
            end

            attr_accessor :name
        end

        T_ObjectAdapterDeactivatedException = ::Ice::__defineException('::Ice::ObjectAdapterDeactivatedException', ObjectAdapterDeactivatedException, false, nil, [["name", ::Ice::T_string, false, 0]])
    end

    if not defined?(::Ice::ObjectAdapterIdInUseException)
        class ObjectAdapterIdInUseException < Ice::LocalException
            def initialize(id='')
                @id = id
            end

            def to_s
                '::Ice::ObjectAdapterIdInUseException'
            end

            attr_accessor :id
        end

        T_ObjectAdapterIdInUseException = ::Ice::__defineException('::Ice::ObjectAdapterIdInUseException', ObjectAdapterIdInUseException, false, nil, [["id", ::Ice::T_string, false, 0]])
    end

    if not defined?(::Ice::NoEndpointException)
        class NoEndpointException < Ice::LocalException
            def initialize(proxy='')
                @proxy = proxy
            end

            def to_s
                '::Ice::NoEndpointException'
            end

            attr_accessor :proxy
        end

        T_NoEndpointException = ::Ice::__defineException('::Ice::NoEndpointException', NoEndpointException, false, nil, [["proxy", ::Ice::T_string, false, 0]])
    end

    if not defined?(::Ice::EndpointParseException)
        class EndpointParseException < Ice::LocalException
            def initialize(str='')
                @str = str
            end

            def to_s
                '::Ice::EndpointParseException'
            end

            attr_accessor :str
        end

        T_EndpointParseException = ::Ice::__defineException('::Ice::EndpointParseException', EndpointParseException, false, nil, [["str", ::Ice::T_string, false, 0]])
    end

    if not defined?(::Ice::EndpointSelectionTypeParseException)
        class EndpointSelectionTypeParseException < Ice::LocalException
            def initialize(str='')
                @str = str
            end

            def to_s
                '::Ice::EndpointSelectionTypeParseException'
            end

            attr_accessor :str
        end

        T_EndpointSelectionTypeParseException = ::Ice::__defineException('::Ice::EndpointSelectionTypeParseException', EndpointSelectionTypeParseException, false, nil, [["str", ::Ice::T_string, false, 0]])
    end

    if not defined?(::Ice::VersionParseException)
        class VersionParseException < Ice::LocalException
            def initialize(str='')
                @str = str
            end

            def to_s
                '::Ice::VersionParseException'
            end

            attr_accessor :str
        end

        T_VersionParseException = ::Ice::__defineException('::Ice::VersionParseException', VersionParseException, false, nil, [["str", ::Ice::T_string, false, 0]])
    end

    if not defined?(::Ice::IdentityParseException)
        class IdentityParseException < Ice::LocalException
            def initialize(str='')
                @str = str
            end

            def to_s
                '::Ice::IdentityParseException'
            end

            attr_accessor :str
        end

        T_IdentityParseException = ::Ice::__defineException('::Ice::IdentityParseException', IdentityParseException, false, nil, [["str", ::Ice::T_string, false, 0]])
    end

    if not defined?(::Ice::ProxyParseException)
        class ProxyParseException < Ice::LocalException
            def initialize(str='')
                @str = str
            end

            def to_s
                '::Ice::ProxyParseException'
            end

            attr_accessor :str
        end

        T_ProxyParseException = ::Ice::__defineException('::Ice::ProxyParseException', ProxyParseException, false, nil, [["str", ::Ice::T_string, false, 0]])
    end

    if not defined?(::Ice::IllegalIdentityException)
        class IllegalIdentityException < Ice::LocalException
            def initialize
            end

            def to_s
                '::Ice::IllegalIdentityException'
            end
        end

        T_IllegalIdentityException = ::Ice::__defineException('::Ice::IllegalIdentityException', IllegalIdentityException, false, nil, [])
    end

    if not defined?(::Ice::IllegalServantException)
        class IllegalServantException < Ice::LocalException
            def initialize(reason='')
                @reason = reason
            end

            def to_s
                '::Ice::IllegalServantException'
            end

            attr_accessor :reason
        end

        T_IllegalServantException = ::Ice::__defineException('::Ice::IllegalServantException', IllegalServantException, false, nil, [["reason", ::Ice::T_string, false, 0]])
    end

    if not defined?(::Ice::RequestFailedException)
        class RequestFailedException < Ice::LocalException
            def initialize(id=::Ice::Identity.new, facet='', operation='')
                @id = id
                @facet = facet
                @operation = operation
            end

            def to_s
                '::Ice::RequestFailedException'
            end

            attr_accessor :id, :facet, :operation
        end

        T_RequestFailedException = ::Ice::__defineException('::Ice::RequestFailedException', RequestFailedException, false, nil, [
            ["id", ::Ice::T_Identity, false, 0],
            ["facet", ::Ice::T_string, false, 0],
            ["operation", ::Ice::T_string, false, 0]
        ])
    end

    if not defined?(::Ice::ObjectNotExistException)
        class ObjectNotExistException < ::Ice::RequestFailedException
            def initialize(id=::Ice::Identity.new, facet='', operation='')
                super(id, facet, operation)
            end

            def to_s
                '::Ice::ObjectNotExistException'
            end
        end

        T_ObjectNotExistException = ::Ice::__defineException('::Ice::ObjectNotExistException', ObjectNotExistException, false, ::Ice::T_RequestFailedException, [])
    end

    if not defined?(::Ice::FacetNotExistException)
        class FacetNotExistException < ::Ice::RequestFailedException
            def initialize(id=::Ice::Identity.new, facet='', operation='')
                super(id, facet, operation)
            end

            def to_s
                '::Ice::FacetNotExistException'
            end
        end

        T_FacetNotExistException = ::Ice::__defineException('::Ice::FacetNotExistException', FacetNotExistException, false, ::Ice::T_RequestFailedException, [])
    end

    if not defined?(::Ice::OperationNotExistException)
        class OperationNotExistException < ::Ice::RequestFailedException
            def initialize(id=::Ice::Identity.new, facet='', operation='')
                super(id, facet, operation)
            end

            def to_s
                '::Ice::OperationNotExistException'
            end
        end

        T_OperationNotExistException = ::Ice::__defineException('::Ice::OperationNotExistException', OperationNotExistException, false, ::Ice::T_RequestFailedException, [])
    end

    if not defined?(::Ice::SyscallException)
        class SyscallException < Ice::LocalException
            def initialize(error=0)
                @error = error
            end

            def to_s
                '::Ice::SyscallException'
            end

            attr_accessor :error
        end

        T_SyscallException = ::Ice::__defineException('::Ice::SyscallException', SyscallException, false, nil, [["error", ::Ice::T_int, false, 0]])
    end

    if not defined?(::Ice::SocketException)
        class SocketException < ::Ice::SyscallException
            def initialize(error=0)
                super(error)
            end

            def to_s
                '::Ice::SocketException'
            end
        end

        T_SocketException = ::Ice::__defineException('::Ice::SocketException', SocketException, false, ::Ice::T_SyscallException, [])
    end

    if not defined?(::Ice::CFNetworkException)
        class CFNetworkException < ::Ice::SocketException
            def initialize(error=0, domain='')
                super(error)
                @domain = domain
            end

            def to_s
                '::Ice::CFNetworkException'
            end

            attr_accessor :domain
        end

        T_CFNetworkException = ::Ice::__defineException('::Ice::CFNetworkException', CFNetworkException, false, ::Ice::T_SocketException, [["domain", ::Ice::T_string, false, 0]])
    end

    if not defined?(::Ice::FileException)
        class FileException < ::Ice::SyscallException
            def initialize(error=0, path='')
                super(error)
                @path = path
            end

            def to_s
                '::Ice::FileException'
            end

            attr_accessor :path
        end

        T_FileException = ::Ice::__defineException('::Ice::FileException', FileException, false, ::Ice::T_SyscallException, [["path", ::Ice::T_string, false, 0]])
    end

    if not defined?(::Ice::ConnectFailedException)
        class ConnectFailedException < ::Ice::SocketException
            def initialize(error=0)
                super(error)
            end

            def to_s
                '::Ice::ConnectFailedException'
            end
        end

        T_ConnectFailedException = ::Ice::__defineException('::Ice::ConnectFailedException', ConnectFailedException, false, ::Ice::T_SocketException, [])
    end

    if not defined?(::Ice::ConnectionRefusedException)
        class ConnectionRefusedException < ::Ice::ConnectFailedException
            def initialize(error=0)
                super(error)
            end

            def to_s
                '::Ice::ConnectionRefusedException'
            end
        end

        T_ConnectionRefusedException = ::Ice::__defineException('::Ice::ConnectionRefusedException', ConnectionRefusedException, false, ::Ice::T_ConnectFailedException, [])
    end

    if not defined?(::Ice::ConnectionLostException)
        class ConnectionLostException < ::Ice::SocketException
            def initialize(error=0)
                super(error)
            end

            def to_s
                '::Ice::ConnectionLostException'
            end
        end

        T_ConnectionLostException = ::Ice::__defineException('::Ice::ConnectionLostException', ConnectionLostException, false, ::Ice::T_SocketException, [])
    end

    if not defined?(::Ice::DNSException)
        class DNSException < Ice::LocalException
            def initialize(error=0, host='')
                @error = error
                @host = host
            end

            def to_s
                '::Ice::DNSException'
            end

            attr_accessor :error, :host
        end

        T_DNSException = ::Ice::__defineException('::Ice::DNSException', DNSException, false, nil, [
            ["error", ::Ice::T_int, false, 0],
            ["host", ::Ice::T_string, false, 0]
        ])
    end

    if not defined?(::Ice::OperationInterruptedException)
        class OperationInterruptedException < Ice::LocalException
            def initialize
            end

            def to_s
                '::Ice::OperationInterruptedException'
            end
        end

        T_OperationInterruptedException = ::Ice::__defineException('::Ice::OperationInterruptedException', OperationInterruptedException, false, nil, [])
    end

    if not defined?(::Ice::TimeoutException)
        class TimeoutException < Ice::LocalException
            def initialize
            end

            def to_s
                '::Ice::TimeoutException'
            end
        end

        T_TimeoutException = ::Ice::__defineException('::Ice::TimeoutException', TimeoutException, false, nil, [])
    end

    if not defined?(::Ice::ConnectTimeoutException)
        class ConnectTimeoutException < ::Ice::TimeoutException
            def initialize
            end

            def to_s
                '::Ice::ConnectTimeoutException'
            end
        end

        T_ConnectTimeoutException = ::Ice::__defineException('::Ice::ConnectTimeoutException', ConnectTimeoutException, false, ::Ice::T_TimeoutException, [])
    end

    if not defined?(::Ice::CloseTimeoutException)
        class CloseTimeoutException < ::Ice::TimeoutException
            def initialize
            end

            def to_s
                '::Ice::CloseTimeoutException'
            end
        end

        T_CloseTimeoutException = ::Ice::__defineException('::Ice::CloseTimeoutException', CloseTimeoutException, false, ::Ice::T_TimeoutException, [])
    end

    if not defined?(::Ice::ConnectionTimeoutException)
        class ConnectionTimeoutException < ::Ice::TimeoutException
            def initialize
            end

            def to_s
                '::Ice::ConnectionTimeoutException'
            end
        end

        T_ConnectionTimeoutException = ::Ice::__defineException('::Ice::ConnectionTimeoutException', ConnectionTimeoutException, false, ::Ice::T_TimeoutException, [])
    end

    if not defined?(::Ice::InvocationTimeoutException)
        class InvocationTimeoutException < ::Ice::TimeoutException
            def initialize
            end

            def to_s
                '::Ice::InvocationTimeoutException'
            end
        end

        T_InvocationTimeoutException = ::Ice::__defineException('::Ice::InvocationTimeoutException', InvocationTimeoutException, false, ::Ice::T_TimeoutException, [])
    end

    if not defined?(::Ice::InvocationCanceledException)
        class InvocationCanceledException < Ice::LocalException
            def initialize
            end

            def to_s
                '::Ice::InvocationCanceledException'
            end
        end

        T_InvocationCanceledException = ::Ice::__defineException('::Ice::InvocationCanceledException', InvocationCanceledException, false, nil, [])
    end

    if not defined?(::Ice::ProtocolException)
        class ProtocolException < Ice::LocalException
            def initialize(reason='')
                @reason = reason
            end

            def to_s
                '::Ice::ProtocolException'
            end

            attr_accessor :reason
        end

        T_ProtocolException = ::Ice::__defineException('::Ice::ProtocolException', ProtocolException, false, nil, [["reason", ::Ice::T_string, false, 0]])
    end

    if not defined?(::Ice::BadMagicException)
        class BadMagicException < ::Ice::ProtocolException
            def initialize(reason='', badMagic=nil)
                super(reason)
                @badMagic = badMagic
            end

            def to_s
                '::Ice::BadMagicException'
            end

            attr_accessor :badMagic
        end

        T_BadMagicException = ::Ice::__defineException('::Ice::BadMagicException', BadMagicException, false, ::Ice::T_ProtocolException, [["badMagic", ::Ice::T_ByteSeq, false, 0]])
    end

    if not defined?(::Ice::UnsupportedProtocolException)
        class UnsupportedProtocolException < ::Ice::ProtocolException
            def initialize(reason='', bad=::Ice::ProtocolVersion.new, supported=::Ice::ProtocolVersion.new)
                super(reason)
                @bad = bad
                @supported = supported
            end

            def to_s
                '::Ice::UnsupportedProtocolException'
            end

            attr_accessor :bad, :supported
        end

        T_UnsupportedProtocolException = ::Ice::__defineException('::Ice::UnsupportedProtocolException', UnsupportedProtocolException, false, ::Ice::T_ProtocolException, [
            ["bad", ::Ice::T_ProtocolVersion, false, 0],
            ["supported", ::Ice::T_ProtocolVersion, false, 0]
        ])
    end

    if not defined?(::Ice::UnsupportedEncodingException)
        class UnsupportedEncodingException < ::Ice::ProtocolException
            def initialize(reason='', bad=::Ice::EncodingVersion.new, supported=::Ice::EncodingVersion.new)
                super(reason)
                @bad = bad
                @supported = supported
            end

            def to_s
                '::Ice::UnsupportedEncodingException'
            end

            attr_accessor :bad, :supported
        end

        T_UnsupportedEncodingException = ::Ice::__defineException('::Ice::UnsupportedEncodingException', UnsupportedEncodingException, false, ::Ice::T_ProtocolException, [
            ["bad", ::Ice::T_EncodingVersion, false, 0],
            ["supported", ::Ice::T_EncodingVersion, false, 0]
        ])
    end

    if not defined?(::Ice::UnknownMessageException)
        class UnknownMessageException < ::Ice::ProtocolException
            def initialize(reason='')
                super(reason)
            end

            def to_s
                '::Ice::UnknownMessageException'
            end
        end

        T_UnknownMessageException = ::Ice::__defineException('::Ice::UnknownMessageException', UnknownMessageException, false, ::Ice::T_ProtocolException, [])
    end

    if not defined?(::Ice::ConnectionNotValidatedException)
        class ConnectionNotValidatedException < ::Ice::ProtocolException
            def initialize(reason='')
                super(reason)
            end

            def to_s
                '::Ice::ConnectionNotValidatedException'
            end
        end

        T_ConnectionNotValidatedException = ::Ice::__defineException('::Ice::ConnectionNotValidatedException', ConnectionNotValidatedException, false, ::Ice::T_ProtocolException, [])
    end

    if not defined?(::Ice::UnknownRequestIdException)
        class UnknownRequestIdException < ::Ice::ProtocolException
            def initialize(reason='')
                super(reason)
            end

            def to_s
                '::Ice::UnknownRequestIdException'
            end
        end

        T_UnknownRequestIdException = ::Ice::__defineException('::Ice::UnknownRequestIdException', UnknownRequestIdException, false, ::Ice::T_ProtocolException, [])
    end

    if not defined?(::Ice::UnknownReplyStatusException)
        class UnknownReplyStatusException < ::Ice::ProtocolException
            def initialize(reason='')
                super(reason)
            end

            def to_s
                '::Ice::UnknownReplyStatusException'
            end
        end

        T_UnknownReplyStatusException = ::Ice::__defineException('::Ice::UnknownReplyStatusException', UnknownReplyStatusException, false, ::Ice::T_ProtocolException, [])
    end

    if not defined?(::Ice::CloseConnectionException)
        class CloseConnectionException < ::Ice::ProtocolException
            def initialize(reason='')
                super(reason)
            end

            def to_s
                '::Ice::CloseConnectionException'
            end
        end

        T_CloseConnectionException = ::Ice::__defineException('::Ice::CloseConnectionException', CloseConnectionException, false, ::Ice::T_ProtocolException, [])
    end

    if not defined?(::Ice::ConnectionManuallyClosedException)
        class ConnectionManuallyClosedException < Ice::LocalException
            def initialize(graceful=false)
                @graceful = graceful
            end

            def to_s
                '::Ice::ConnectionManuallyClosedException'
            end

            attr_accessor :graceful
        end

        T_ConnectionManuallyClosedException = ::Ice::__defineException('::Ice::ConnectionManuallyClosedException', ConnectionManuallyClosedException, false, nil, [["graceful", ::Ice::T_bool, false, 0]])
    end

    if not defined?(::Ice::IllegalMessageSizeException)
        class IllegalMessageSizeException < ::Ice::ProtocolException
            def initialize(reason='')
                super(reason)
            end

            def to_s
                '::Ice::IllegalMessageSizeException'
            end
        end

        T_IllegalMessageSizeException = ::Ice::__defineException('::Ice::IllegalMessageSizeException', IllegalMessageSizeException, false, ::Ice::T_ProtocolException, [])
    end

    if not defined?(::Ice::CompressionException)
        class CompressionException < ::Ice::ProtocolException
            def initialize(reason='')
                super(reason)
            end

            def to_s
                '::Ice::CompressionException'
            end
        end

        T_CompressionException = ::Ice::__defineException('::Ice::CompressionException', CompressionException, false, ::Ice::T_ProtocolException, [])
    end

    if not defined?(::Ice::DatagramLimitException)
        class DatagramLimitException < ::Ice::ProtocolException
            def initialize(reason='')
                super(reason)
            end

            def to_s
                '::Ice::DatagramLimitException'
            end
        end

        T_DatagramLimitException = ::Ice::__defineException('::Ice::DatagramLimitException', DatagramLimitException, false, ::Ice::T_ProtocolException, [])
    end

    if not defined?(::Ice::MarshalException)
        class MarshalException < ::Ice::ProtocolException
            def initialize(reason='')
                super(reason)
            end

            def to_s
                '::Ice::MarshalException'
            end
        end

        T_MarshalException = ::Ice::__defineException('::Ice::MarshalException', MarshalException, false, ::Ice::T_ProtocolException, [])
    end

    if not defined?(::Ice::ProxyUnmarshalException)
        class ProxyUnmarshalException < ::Ice::MarshalException
            def initialize(reason='')
                super(reason)
            end

            def to_s
                '::Ice::ProxyUnmarshalException'
            end
        end

        T_ProxyUnmarshalException = ::Ice::__defineException('::Ice::ProxyUnmarshalException', ProxyUnmarshalException, false, ::Ice::T_MarshalException, [])
    end

    if not defined?(::Ice::UnmarshalOutOfBoundsException)
        class UnmarshalOutOfBoundsException < ::Ice::MarshalException
            def initialize(reason='')
                super(reason)
            end

            def to_s
                '::Ice::UnmarshalOutOfBoundsException'
            end
        end

        T_UnmarshalOutOfBoundsException = ::Ice::__defineException('::Ice::UnmarshalOutOfBoundsException', UnmarshalOutOfBoundsException, false, ::Ice::T_MarshalException, [])
    end

    if not defined?(::Ice::NoValueFactoryException)
        class NoValueFactoryException < ::Ice::MarshalException
            def initialize(reason='', type='')
                super(reason)
                @type = type
            end

            def to_s
                '::Ice::NoValueFactoryException'
            end

            attr_accessor :type
        end

        T_NoValueFactoryException = ::Ice::__defineException('::Ice::NoValueFactoryException', NoValueFactoryException, false, ::Ice::T_MarshalException, [["type", ::Ice::T_string, false, 0]])
    end

    if not defined?(::Ice::UnexpectedObjectException)
        class UnexpectedObjectException < ::Ice::MarshalException
            def initialize(reason='', type='', expectedType='')
                super(reason)
                @type = type
                @expectedType = expectedType
            end

            def to_s
                '::Ice::UnexpectedObjectException'
            end

            attr_accessor :type, :expectedType
        end

        T_UnexpectedObjectException = ::Ice::__defineException('::Ice::UnexpectedObjectException', UnexpectedObjectException, false, ::Ice::T_MarshalException, [
            ["type", ::Ice::T_string, false, 0],
            ["expectedType", ::Ice::T_string, false, 0]
        ])
    end

    if not defined?(::Ice::MemoryLimitException)
        class MemoryLimitException < ::Ice::MarshalException
            def initialize(reason='')
                super(reason)
            end

            def to_s
                '::Ice::MemoryLimitException'
            end
        end

        T_MemoryLimitException = ::Ice::__defineException('::Ice::MemoryLimitException', MemoryLimitException, false, ::Ice::T_MarshalException, [])
    end

    if not defined?(::Ice::StringConversionException)
        class StringConversionException < ::Ice::MarshalException
            def initialize(reason='')
                super(reason)
            end

            def to_s
                '::Ice::StringConversionException'
            end
        end

        T_StringConversionException = ::Ice::__defineException('::Ice::StringConversionException', StringConversionException, false, ::Ice::T_MarshalException, [])
    end

    if not defined?(::Ice::EncapsulationException)
        class EncapsulationException < ::Ice::MarshalException
            def initialize(reason='')
                super(reason)
            end

            def to_s
                '::Ice::EncapsulationException'
            end
        end

        T_EncapsulationException = ::Ice::__defineException('::Ice::EncapsulationException', EncapsulationException, false, ::Ice::T_MarshalException, [])
    end

    if not defined?(::Ice::FeatureNotSupportedException)
        class FeatureNotSupportedException < Ice::LocalException
            def initialize(unsupportedFeature='')
                @unsupportedFeature = unsupportedFeature
            end

            def to_s
                '::Ice::FeatureNotSupportedException'
            end

            attr_accessor :unsupportedFeature
        end

        T_FeatureNotSupportedException = ::Ice::__defineException('::Ice::FeatureNotSupportedException', FeatureNotSupportedException, false, nil, [["unsupportedFeature", ::Ice::T_string, false, 0]])
    end

    if not defined?(::Ice::SecurityException)
        class SecurityException < Ice::LocalException
            def initialize(reason='')
                @reason = reason
            end

            def to_s
                '::Ice::SecurityException'
            end

            attr_accessor :reason
        end

        T_SecurityException = ::Ice::__defineException('::Ice::SecurityException', SecurityException, false, nil, [["reason", ::Ice::T_string, false, 0]])
    end

    if not defined?(::Ice::FixedProxyException)
        class FixedProxyException < Ice::LocalException
            def initialize
            end

            def to_s
                '::Ice::FixedProxyException'
            end
        end

        T_FixedProxyException = ::Ice::__defineException('::Ice::FixedProxyException', FixedProxyException, false, nil, [])
    end

    if not defined?(::Ice::ResponseSentException)
        class ResponseSentException < Ice::LocalException
            def initialize
            end

            def to_s
                '::Ice::ResponseSentException'
            end
        end

        T_ResponseSentException = ::Ice::__defineException('::Ice::ResponseSentException', ResponseSentException, false, nil, [])
    end
end
