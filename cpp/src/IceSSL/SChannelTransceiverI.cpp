//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include <IceSSL/SChannelTransceiverI.h>

#include <IceUtil/StringUtil.h>

#include <IceSSL/ConnectionInfo.h>
#include <IceSSL/Instance.h>
#include <IceSSL/SChannelEngine.h>
#include <IceSSL/PluginI.h>
#include <IceSSL/Util.h>
#include <Ice/Communicator.h>
#include <Ice/LoggerUtil.h>
#include <Ice/Buffer.h>
#include <Ice/LocalException.h>

using namespace std;
using namespace Ice;
using namespace IceSSL;

#ifndef CERT_CHAIN_DISABLE_AIA
#   define CERT_CHAIN_DISABLE_AIA 0x00002000
#endif

namespace
{

string
protocolName(DWORD protocol)
{
    switch(protocol)
    {
        case SP_PROT_SSL2_CLIENT:
        case SP_PROT_SSL2_SERVER:
            return "SSL 2.0";
        case SP_PROT_SSL3_CLIENT:
        case SP_PROT_SSL3_SERVER:
            return "SSL 3.0";
        case SP_PROT_TLS1_CLIENT:
        case SP_PROT_TLS1_SERVER:
            return "TLS 1.0";
        case SP_PROT_TLS1_1_CLIENT:
        case SP_PROT_TLS1_1_SERVER:
            return "TLS 1.1";
        case SP_PROT_TLS1_2_CLIENT:
        case SP_PROT_TLS1_2_SERVER:
            return "TLS 1.2";
        default:
            return "Unknown";
    }
}

TrustError
trustStatusToTrustError(DWORD status)
{
    if (status & CERT_TRUST_NO_ERROR)
    {
        return IceSSL::TrustError::NoError;
    }
    if ((status & CERT_TRUST_IS_UNTRUSTED_ROOT) ||
        (status & CERT_TRUST_IS_CYCLIC) ||
        (status & CERT_TRUST_CTL_IS_NOT_TIME_VALID) ||
        (status & CERT_TRUST_CTL_IS_NOT_SIGNATURE_VALID) ||
        (status & CERT_TRUST_CTL_IS_NOT_VALID_FOR_USAGE))
    {
        return IceSSL::TrustError::UntrustedRoot;
    }
    if (status & CERT_TRUST_IS_EXPLICIT_DISTRUST)
    {
        return IceSSL::TrustError::NotTrusted;
    }
    if (status & CERT_TRUST_IS_PARTIAL_CHAIN)
    {
        return IceSSL::TrustError::PartialChain;
    }
    if (status & CERT_TRUST_INVALID_BASIC_CONSTRAINTS)
    {
        return IceSSL::TrustError::InvalidBasicConstraints;
    }
    if (status & CERT_TRUST_IS_NOT_SIGNATURE_VALID)
    {
        return IceSSL::TrustError::InvalidSignature;
    }
    if (status & CERT_TRUST_IS_NOT_VALID_FOR_USAGE)
    {
        return IceSSL::TrustError::InvalidPurpose;
    }
    if (status & CERT_TRUST_IS_REVOKED)
    {
        return IceSSL::TrustError::Revoked;
    }
    if (status & CERT_TRUST_INVALID_EXTENSION)
    {
        return IceSSL::TrustError::InvalidExtension;
    }
    if (status & CERT_TRUST_INVALID_POLICY_CONSTRAINTS)
    {
        return IceSSL::TrustError::InvalidPolicyConstraints;
    }
    if (status & CERT_TRUST_INVALID_NAME_CONSTRAINTS)
    {
        return IceSSL::TrustError::InvalidNameConstraints;
    }
    if (status & CERT_TRUST_HAS_NOT_SUPPORTED_NAME_CONSTRAINT)
    {
        return IceSSL::TrustError::HasNonSupportedNameConstraint;
    }
    if (status & CERT_TRUST_HAS_NOT_DEFINED_NAME_CONSTRAINT)
    {
        return IceSSL::TrustError::HasNonDefinedNameConstraint;
    }
    if (status & CERT_TRUST_HAS_NOT_PERMITTED_NAME_CONSTRAINT)
    {
        return IceSSL::TrustError::HasNonPermittedNameConstraint;
    }
    if (status & CERT_TRUST_HAS_EXCLUDED_NAME_CONSTRAINT)
    {
        return IceSSL::TrustError::HasExcludedNameConstraint;
    }
    if (status & CERT_TRUST_NO_ISSUANCE_CHAIN_POLICY)
    {
        return IceSSL::TrustError::InvalidPolicyConstraints;
    }
    if (status & CERT_TRUST_HAS_NOT_SUPPORTED_CRITICAL_EXT)
    {
        return IceSSL::TrustError::HasNonSupportedCriticalExtension;
    }
    if (status & CERT_TRUST_IS_OFFLINE_REVOCATION ||
        status & CERT_TRUST_REVOCATION_STATUS_UNKNOWN)
    {
        return IceSSL::TrustError::RevocationStatusUnknown;
    }
    if (status & CERT_TRUST_IS_NOT_TIME_VALID)
    {
        return IceSSL::TrustError::InvalidTime;
    }
    return IceSSL::TrustError::UnknownTrustFailure;
}

string
trustStatusToString(DWORD status)
{
    if(status & CERT_TRUST_NO_ERROR)
    {
        return "CERT_TRUST_NO_ERROR";
    }

    if(status & CERT_TRUST_IS_NOT_TIME_VALID)
    {
        return "CERT_TRUST_IS_NOT_TIME_VALID";
    }

    if(status & CERT_TRUST_IS_REVOKED)
    {
        return "CERT_TRUST_IS_REVOKED";
    }

    if(status & CERT_TRUST_IS_NOT_SIGNATURE_VALID)
    {
        return "CERT_TRUST_IS_NOT_SIGNATURE_VALID";
    }

    if(status & CERT_TRUST_IS_NOT_VALID_FOR_USAGE)
    {
        return "CERT_TRUST_IS_NOT_VALID_FOR_USAGE";
    }

    if(status & CERT_TRUST_IS_UNTRUSTED_ROOT)
    {
        return "CERT_TRUST_IS_UNTRUSTED_ROOT";
    }

    if(status & CERT_TRUST_REVOCATION_STATUS_UNKNOWN)
    {
        return "CERT_TRUST_REVOCATION_STATUS_UNKNOWN";
    }

    if(status & CERT_TRUST_IS_CYCLIC)
    {
        return "CERT_TRUST_IS_CYCLIC";
    }

    if(status & CERT_TRUST_INVALID_EXTENSION)
    {
        return "CERT_TRUST_INVALID_EXTENSION";
    }

    if(status & CERT_TRUST_INVALID_POLICY_CONSTRAINTS)
    {
        return "CERT_TRUST_INVALID_POLICY_CONSTRAINTS";
    }

    if(status & CERT_TRUST_INVALID_BASIC_CONSTRAINTS)
    {
        return "CERT_TRUST_INVALID_BASIC_CONSTRAINTS";
    }

    if(status & CERT_TRUST_INVALID_NAME_CONSTRAINTS)
    {
        return "CERT_TRUST_INVALID_NAME_CONSTRAINTS";
    }

    if(status & CERT_TRUST_HAS_NOT_SUPPORTED_NAME_CONSTRAINT)
    {
        return "CERT_TRUST_HAS_NOT_SUPPORTED_NAME_CONSTRAINT";
    }

    if(status & CERT_TRUST_HAS_NOT_DEFINED_NAME_CONSTRAINT)
    {
        return "CERT_TRUST_HAS_NOT_DEFINED_NAME_CONSTRAINT";
    }

    if(status & CERT_TRUST_HAS_NOT_PERMITTED_NAME_CONSTRAINT)
    {
        return "CERT_TRUST_HAS_NOT_PERMITTED_NAME_CONSTRAINT";
    }

    if(status & CERT_TRUST_HAS_EXCLUDED_NAME_CONSTRAINT)
    {
        return "CERT_TRUST_HAS_EXCLUDED_NAME_CONSTRAINT";
    }

    if(status & CERT_TRUST_IS_OFFLINE_REVOCATION)
    {
        return "CERT_TRUST_IS_OFFLINE_REVOCATION";
    }

    if(status & CERT_TRUST_NO_ISSUANCE_CHAIN_POLICY)
    {
        return "CERT_TRUST_NO_ISSUANCE_CHAIN_POLICY";
    }

    if(status & CERT_TRUST_IS_EXPLICIT_DISTRUST)
    {
        return "CERT_TRUST_IS_EXPLICIT_DISTRUST";
    }

    if(status & CERT_TRUST_HAS_NOT_SUPPORTED_CRITICAL_EXT)
    {
        return "CERT_TRUST_HAS_NOT_SUPPORTED_CRITICAL_EXT";
    }

    //
    // New in Windows 8
    //
    //if(status & CERT_TRUST_HAS_WEAK_SIGNATURE)
    //{
    //    return "CERT_TRUST_HAS_WEAK_SIGNATURE";
    //}

    if(status & CERT_TRUST_IS_PARTIAL_CHAIN)
    {
        return "CERT_TRUST_IS_PARTIAL_CHAIN";
    }

    if(status & CERT_TRUST_CTL_IS_NOT_TIME_VALID)
    {
        return "CERT_TRUST_CTL_IS_NOT_TIME_VALID";
    }

    if(status & CERT_TRUST_CTL_IS_NOT_SIGNATURE_VALID)
    {
        return "CERT_TRUST_CTL_IS_NOT_SIGNATURE_VALID";
    }

    if(status & CERT_TRUST_CTL_IS_NOT_VALID_FOR_USAGE)
    {
        return "CERT_TRUST_CTL_IS_NOT_VALID_FOR_USAGE";
    }

    ostringstream os;
    os << "UNKNOWN TRUST FAILURE: " << status;
    return os.str();
}

SecBuffer*
getSecBufferWithType(const SecBufferDesc& desc, ULONG bufferType)
{
    for(ULONG i = 0; i < desc.cBuffers; ++i)
    {
        if(desc.pBuffers[i].BufferType == bufferType)
        {
            return &desc.pBuffers[i];
        }
    }
    return 0;
}

string
secStatusToString(SECURITY_STATUS status)
{
    return IceUtilInternal::errorToString(status);
}

}

IceInternal::NativeInfoPtr
SChannel::TransceiverI::getNativeInfo()
{
    return _delegate->getNativeInfo();
}

IceInternal::SocketOperation
SChannel::TransceiverI::sslHandshake()
{
    DWORD flags = ASC_REQ_SEQUENCE_DETECT | ASC_REQ_REPLAY_DETECT | ASC_REQ_CONFIDENTIALITY | ASC_REQ_ALLOCATE_MEMORY |
        ASC_REQ_STREAM;
    if(_incoming)
    {
        flags |= ASC_REQ_EXTENDED_ERROR;
        if(_engine->getVerifyPeer() > 0)
        {
            flags |= ASC_REQ_MUTUAL_AUTH;
        }
    }
    else
    {
        flags |= ISC_REQ_USE_SUPPLIED_CREDS | ISC_REQ_MANUAL_CRED_VALIDATION | ISC_RET_EXTENDED_ERROR;
    }

    SECURITY_STATUS err = SEC_E_OK;
    if(_state == StateHandshakeNotStarted)
    {
        _ctxFlags = 0;
        _readBuffer.b.resize(2048);
        _readBuffer.i = _readBuffer.b.begin();
        _credentials = _engine->newCredentialsHandle(_incoming);
        _credentialsInitialized = true;

        if(!_incoming)
        {
            SecBuffer outBuffer = { 0, SECBUFFER_TOKEN, 0 };
            SecBufferDesc outBufferDesc = { SECBUFFER_VERSION, 1, &outBuffer };

            err = InitializeSecurityContext(&_credentials, 0, const_cast<char *>(_host.c_str()), flags, 0, 0, 0, 0,
                                            &_ssl, &outBufferDesc, &_ctxFlags, 0);
            if(err != SEC_E_OK && err != SEC_I_CONTINUE_NEEDED)
            {
                throw SecurityException(__FILE__, __LINE__, "IceSSL: handshake failure:\n" + secStatusToString(err));
            }
            _sslInitialized = true;

            //
            // Copy the data to the write buffer
            //
            _writeBuffer.b.resize(outBuffer.cbBuffer);
            _writeBuffer.i = _writeBuffer.b.begin();
            memcpy(_writeBuffer.i, outBuffer.pvBuffer, outBuffer.cbBuffer);
            FreeContextBuffer(outBuffer.pvBuffer);

            _state = StateHandshakeWriteContinue;
        }
        else
        {
            _state = StateHandshakeReadContinue;
        }
    }

    while(true)
    {
        if(_state == StateHandshakeReadContinue)
        {
            // If read buffer is empty, try to read some data.
            if(_readBuffer.i == _readBuffer.b.begin() && !readRaw(_readBuffer))
            {
                return IceInternal::SocketOperationRead;
            }

            SecBuffer inBuffers[2] = {
                { static_cast<DWORD>(_readBuffer.i - _readBuffer.b.begin()), SECBUFFER_TOKEN, _readBuffer.b.begin() },
                { 0, SECBUFFER_EMPTY, 0 }
            };
            SecBufferDesc inBufferDesc = { SECBUFFER_VERSION, 2, inBuffers };

            SecBuffer outBuffers[2] = {
                { 0, SECBUFFER_TOKEN, 0 },
                { 0, SECBUFFER_ALERT, 0 }
            };
            SecBufferDesc outBufferDesc = { SECBUFFER_VERSION, 2, outBuffers };

            if(_incoming)
            {
                err = AcceptSecurityContext(&_credentials, (_sslInitialized ? &_ssl : 0), &inBufferDesc, flags, 0,
                                            &_ssl, &outBufferDesc, &_ctxFlags, 0);
                if(err == SEC_I_CONTINUE_NEEDED || err == SEC_E_OK)
                {
                    _sslInitialized = true;
                }
            }
            else
            {
                err = InitializeSecurityContext(&_credentials, &_ssl, const_cast<char*>(_host.c_str()), flags, 0, 0,
                                                &inBufferDesc, 0, 0, &outBufferDesc, &_ctxFlags, 0);
            }

            //
            // If the message is incomplete we need to read more data.
            //
            if(err == SEC_E_INCOMPLETE_MESSAGE)
            {
                SecBuffer* missing = getSecBufferWithType(inBufferDesc, SECBUFFER_MISSING);
                size_t pos = _readBuffer.i - _readBuffer.b.begin();
                _readBuffer.b.resize((missing && missing->cbBuffer > 0) ? (pos + missing->cbBuffer) : (pos * 2));
                _readBuffer.i = _readBuffer.b.begin() + pos;
                return IceInternal::SocketOperationRead;
            }
            else if(err != SEC_I_CONTINUE_NEEDED && err != SEC_E_OK)
            {
                throw SecurityException(__FILE__, __LINE__, "SSL handshake failure:\n" + secStatusToString(err));
            }

            //
            // Copy out security tokens to the write buffer if any.
            //
            SecBuffer* token = getSecBufferWithType(outBufferDesc, SECBUFFER_TOKEN);
            assert(token);
            if(token->cbBuffer)
            {
                _writeBuffer.b.resize(static_cast<size_t>(token->cbBuffer));
                _writeBuffer.i = _writeBuffer.b.begin();
                memcpy(_writeBuffer.i, token->pvBuffer, token->cbBuffer);
                FreeContextBuffer(token->pvBuffer);
            }

            //
            // Check for remaining data in the input buffer.
            //
            SecBuffer* extra = getSecBufferWithType(inBufferDesc, SECBUFFER_EXTRA);
            if(extra)
            {
                // Shift the extra data to the start of the input buffer
                memmove(_readBuffer.b.begin(), _readBuffer.i - extra->cbBuffer, extra->cbBuffer);
                _readBuffer.i = _readBuffer.b.begin() + extra->cbBuffer;
            }
            else
            {
                _readBuffer.i = _readBuffer.b.begin();
            }

            if(token->cbBuffer)
            {
                if(err == SEC_E_OK)
                {
                    _state = StateHandshakeWriteNoContinue; // Write and don't continue.
                }
                else
                {
                    _state = StateHandshakeWriteContinue; // Continue writing if we have a token.
                }
            }
            else if(err == SEC_E_OK)
            {
                break; // We're done.
            }

            // Otherwise continue either reading credentials
        }

        if(_state == StateHandshakeWriteContinue || _state == StateHandshakeWriteNoContinue)
        {
            //
            // Write any pending data.
            //
            if(!writeRaw(_writeBuffer))
            {
                return IceInternal::SocketOperationWrite;
            }
            if(_state == StateHandshakeWriteNoContinue)
            {
                break; // Token is written and we weren't told to continue, so we're done!
            }
            _state = StateHandshakeReadContinue;
        }
    }

    //
    // Check if the requested capabilities are met.
    //
    // NOTE: it's important for _ctxFlags to be a data member. The context flags might not be checked immediately
    // if the last write can't complete without blocking above. In such a case, the context flags are checked here
    // only once the sslHandshake is called again after the write completes.
    //
    if(flags != _ctxFlags)
    {
        if(_incoming)
        {
            if(!(_ctxFlags & ASC_REQ_SEQUENCE_DETECT))
            {
                throw SecurityException(__FILE__, __LINE__, "IceSSL: SChannel failed to setup sequence detect");
            }

            if(!(_ctxFlags & ASC_REQ_REPLAY_DETECT))
            {
                throw SecurityException(__FILE__, __LINE__, "IceSSL: SChannel failed to setup replay detect");
            }

            if(!(_ctxFlags & ASC_REQ_CONFIDENTIALITY))
            {
                throw SecurityException(__FILE__, __LINE__, "IceSSL: SChannel failed to setup confidentiality");
            }

            if(!(_ctxFlags & ASC_REQ_EXTENDED_ERROR))
            {
                throw SecurityException(__FILE__, __LINE__, "IceSSL: SChannel failed to setup extended error");
            }

            if(!(_ctxFlags & ASC_REQ_ALLOCATE_MEMORY))
            {
                throw SecurityException(__FILE__, __LINE__, "IceSSL: SChannel failed to setup memory allocation");
            }

            if(!(_ctxFlags & ASC_REQ_STREAM))
            {
                throw SecurityException(__FILE__, __LINE__, "IceSSL: SChannel failed to setup stream");
            }
        }
        else
        {
            if(!(_ctxFlags & ISC_REQ_SEQUENCE_DETECT))
            {
                throw SecurityException(__FILE__, __LINE__, "IceSSL: SChannel failed to setup sequence detect");
            }

            if(!(_ctxFlags & ISC_REQ_REPLAY_DETECT))
            {
                throw SecurityException(__FILE__, __LINE__, "IceSSL: SChannel failed to setup replay detect");
            }

            if(!(_ctxFlags & ISC_REQ_CONFIDENTIALITY))
            {
                throw SecurityException(__FILE__, __LINE__, "IceSSL: SChannel failed to setup confidentiality");
            }

            if(!(_ctxFlags & ISC_REQ_EXTENDED_ERROR))
            {
                throw SecurityException(__FILE__, __LINE__, "IceSSL: SChannel failed to setup extended error");
            }

            if(!(_ctxFlags & ISC_REQ_ALLOCATE_MEMORY))
            {
                throw SecurityException(__FILE__, __LINE__, "IceSSL: SChannel failed to setup memory allocation");
            }

            if(!(_ctxFlags & ISC_REQ_STREAM))
            {
                throw SecurityException(__FILE__, __LINE__, "IceSSL: SChannel failed to setup stream");
            }
        }
    }

    err = QueryContextAttributes(&_ssl, SECPKG_ATTR_STREAM_SIZES, &_sizes);
    if(err != SEC_E_OK)
    {
        throw SecurityException(__FILE__, __LINE__, "IceSSL: failure to query stream sizes attributes:\n" +
                                secStatusToString(err));
    }

    size_t pos = _readBuffer.i - _readBuffer.b.begin();
    if(pos <= (_sizes.cbHeader + _sizes.cbMaximumMessage + _sizes.cbTrailer))
    {
        _readBuffer.b.resize(_sizes.cbHeader + _sizes.cbMaximumMessage + _sizes.cbTrailer);
        _readBuffer.i = _readBuffer.b.begin() + pos;
    }

    _writeBuffer.b.reset();
    _writeBuffer.i = _writeBuffer.b.begin();

    return IceInternal::SocketOperationNone;
}

//
// Try to decrypt a message and return the number of bytes decrypted, if the number of bytes
// decrypted is less than the size requested it means that the application needs to read more
// data before it can decrypt the complete message.
//
size_t
SChannel::TransceiverI::decryptMessage(IceInternal::Buffer& buffer)
{
    assert(_readBuffer.i != _readBuffer.b.begin() || !_readUnprocessed.b.empty());

    //
    // First check if there is data in the unprocessed buffer.
    //
    size_t length = std::min(static_cast<size_t>(buffer.b.end() - buffer.i), _readUnprocessed.b.size());
    if(length > 0)
    {
        memcpy(buffer.i, _readUnprocessed.b.begin(), length);
        memmove(_readUnprocessed.b.begin(), _readUnprocessed.b.begin() + length, _readUnprocessed.b.size() - length);
        _readUnprocessed.b.resize(_readUnprocessed.b.size() - length);
    }

    while(true)
    {
        //
        // If we have filled the buffer or if nothing left to read from
        // the read buffer, we're done.
        //
        Byte* i = buffer.i + length;
        if(i == buffer.b.end() || _readBuffer.i == _readBuffer.b.begin())
        {
            break;
        }

        //
        // Try to decrypt the buffered data.
        //
        SecBuffer inBuffers[4] = {
            { static_cast<DWORD>(_readBuffer.i - _readBuffer.b.begin()), SECBUFFER_DATA, _readBuffer.b.begin() },
            { 0, SECBUFFER_EMPTY, 0 },
            { 0, SECBUFFER_EMPTY, 0 },
            { 0, SECBUFFER_EMPTY, 0 }
        };
        SecBufferDesc inBufferDesc = { SECBUFFER_VERSION, 4, inBuffers };

        SECURITY_STATUS err = DecryptMessage(&_ssl, &inBufferDesc, 0, 0);
        if(err == SEC_E_INCOMPLETE_MESSAGE)
        {
            //
            // There isn't enough data to decrypt the message. The input
            // buffer is resized to the SSL max message size after the SSL
            // handshake completes so an incomplete message can only occur
            // if the read buffer is not full.
            //
            assert(_readBuffer.i != _readBuffer.b.end());
            return length;
        }
        else if(err == SEC_I_CONTEXT_EXPIRED || err == SEC_I_RENEGOTIATE)
        {
            //
            // The message sender has finished using the connection and
            // has initiated a shutdown.
            //
            throw ConnectionLostException(__FILE__, __LINE__, 0);
        }
        else if(err != SEC_E_OK)
        {
            throw ProtocolException(__FILE__, __LINE__, "IceSSL: protocol error during read:\n" +
                                    secStatusToString(err));
        }

        SecBuffer* dataBuffer = getSecBufferWithType(inBufferDesc, SECBUFFER_DATA);
        assert(dataBuffer);
        DWORD remaining = min(static_cast<DWORD>(buffer.b.end() - i), dataBuffer->cbBuffer);
        length += remaining;
        if(remaining)
        {
            memcpy(i, dataBuffer->pvBuffer, remaining);

            //
            // Copy remaining decrypted data to unprocessed buffer
            //
            if(dataBuffer->cbBuffer > remaining)
            {
                _readUnprocessed.b.resize(dataBuffer->cbBuffer - remaining);
                memcpy(_readUnprocessed.b.begin(), reinterpret_cast<Byte*>(dataBuffer->pvBuffer) + remaining,
                    dataBuffer->cbBuffer - remaining);
            }
        }

        //
        // Move any remaining encrypted data to the begining of the input buffer
        //
        SecBuffer* extraBuffer = getSecBufferWithType(inBufferDesc, SECBUFFER_EXTRA);
        if(extraBuffer && extraBuffer->cbBuffer > 0)
        {
            memmove(_readBuffer.b.begin(), _readBuffer.i - extraBuffer->cbBuffer, extraBuffer->cbBuffer);
            _readBuffer.i = _readBuffer.b.begin() + extraBuffer->cbBuffer;
        }
        else
        {
            _readBuffer.i = _readBuffer.b.begin();
        }
    }
    return length;
}

//
// Encrypt a message and return the number of bytes that has been encrypted, if the
// number of bytes is less than the message size, the function must be called again.
//
size_t
SChannel::TransceiverI::encryptMessage(IceInternal::Buffer& buffer)
{
    //
    // Limit the message size to cbMaximumMessage which is the maximun size data that can be
    // embeded in a SSL record.
    //
    DWORD length = std::min(static_cast<DWORD>(buffer.b.end() - buffer.i), _sizes.cbMaximumMessage);

    //
    // Resize the buffer to hold the encrypted data
    //
    _writeBuffer.b.resize(_sizes.cbHeader + length + _sizes.cbTrailer);
    _writeBuffer.i = _writeBuffer.b.begin();

    SecBuffer buffers[4] = {
        { _sizes.cbHeader, SECBUFFER_STREAM_HEADER, _writeBuffer.i },
        { length, SECBUFFER_DATA, _writeBuffer.i + _sizes.cbHeader },
        { _sizes.cbTrailer, SECBUFFER_STREAM_TRAILER, _writeBuffer.i + _sizes.cbHeader + length },
        { 0, SECBUFFER_EMPTY, 0 }
    };
    SecBufferDesc buffersDesc = { SECBUFFER_VERSION, 4, buffers };

    // Data is encrypted in place, copy the data to be encrypted to the data buffer.
    memcpy(buffers[1].pvBuffer, buffer.i, length);

    SECURITY_STATUS err = EncryptMessage(&_ssl, 0, &buffersDesc, 0);
    if(err != SEC_E_OK)
    {
        throw ProtocolException(__FILE__, __LINE__, "IceSSL: protocol error encrypting message:\n" +
                                secStatusToString(err));
    }

    // EncryptMessage resizes the buffers, so resize the write buffer as well to reflect this.
    _writeBuffer.b.resize(buffers[0].cbBuffer + buffers[1].cbBuffer + buffers[2].cbBuffer);
    _writeBuffer.i = _writeBuffer.b.begin();

    return length;
}

IceInternal::SocketOperation
SChannel::TransceiverI::initialize(IceInternal::Buffer& readBuffer, IceInternal::Buffer& writeBuffer)
{
    if(_state == StateNotInitialized)
    {
        IceInternal::SocketOperation op = _delegate->initialize(readBuffer, writeBuffer);
        if(op != IceInternal::SocketOperationNone)
        {
            return op;
        }
        _state = StateHandshakeNotStarted;
    }

    IceInternal::SocketOperation op = sslHandshake();
    if(op != IceInternal::SocketOperationNone)
    {
        return op;
    }

    //
    // Build the peer certificate chain and verify it.
    //
    PCCERT_CONTEXT cert = 0;
    SECURITY_STATUS err = QueryContextAttributes(&_ssl, SECPKG_ATTR_REMOTE_CERT_CONTEXT, &cert);
    if(err && err != SEC_E_NO_CREDENTIALS)
    {
        throw SecurityException(__FILE__, __LINE__, "IceSSL: certificate verification failure:\n" +
                                secStatusToString(err));
    }

    if(!cert && ((!_incoming && _engine->getVerifyPeer() > 0) || (_incoming && _engine->getVerifyPeer() == 2)))
    {
        //
        // Clients require server certificate if VerifyPeer > 0 and servers require client
        // certificate if VerifyPeer == 2
        //
        throw SecurityException(__FILE__, __LINE__, "IceSSL: certificate required");
    }
    else if(cert) // Verify the remote certificate
    {
        CERT_CHAIN_PARA chainP;
        memset(&chainP, 0, sizeof(chainP));
        chainP.cbSize = sizeof(chainP);

        string trustError;
        PCCERT_CHAIN_CONTEXT certChain;
        DWORD dwFlags = 0;
        int revocationCheck = _engine->getRevocationCheck();
        if(revocationCheck > 0)
        {
            if(_engine->getRevocationCheckCacheOnly())
            {
                // Disable network I/O for revocation checks.
                dwFlags = CERT_CHAIN_REVOCATION_CHECK_CACHE_ONLY | CERT_CHAIN_DISABLE_AIA;
            }

            dwFlags |= (revocationCheck == 1 ?
                        CERT_CHAIN_REVOCATION_CHECK_END_CERT :
                        CERT_CHAIN_REVOCATION_CHECK_CHAIN_EXCLUDE_ROOT);
        }

        if(!CertGetCertificateChain(_engine->chainEngine(), cert, 0, cert->hCertStore, &chainP, dwFlags, 0, &certChain))
        {
            CertFreeCertificateContext(cert);
            trustError = IceUtilInternal::lastErrorToString();
            _trustError = IceSSL::TrustError::UnknownTrustFailure;
        }
        else
        {
            if(certChain->TrustStatus.dwErrorStatus != CERT_TRUST_NO_ERROR)
            {
                trustError = trustStatusToString(certChain->TrustStatus.dwErrorStatus);
                _trustError = trustStatusToTrustError(certChain->TrustStatus.dwErrorStatus);
            }
            else
            {
                _verified = true;
                _trustError = IceSSL::TrustError::NoError;
            }

            CERT_SIMPLE_CHAIN* simpleChain = certChain->rgpChain[0];
            for(DWORD i = 0; i < simpleChain->cElement; ++i)
            {
                PCCERT_CONTEXT c = simpleChain->rgpElement[i]->pCertContext;
                PCERT_SIGNED_CONTENT_INFO cc;

                DWORD length = 0;
                if(!CryptDecodeObjectEx(X509_ASN_ENCODING | PKCS_7_ASN_ENCODING, X509_CERT, c->pbCertEncoded,
                                        c->cbCertEncoded, CRYPT_DECODE_ALLOC_FLAG, 0, &cc, &length))
                {
                    CertFreeCertificateChain(certChain);
                    CertFreeCertificateContext(cert);
                    throw SecurityException(__FILE__, __LINE__, "IceSSL: error decoding peer certificate chain:\n" +
                                            IceUtilInternal::lastErrorToString());
                }
                _certs.push_back(SChannel::Certificate::create(cc));
            }

            CertFreeCertificateChain(certChain);
            CertFreeCertificateContext(cert);
        }

        if(!trustError.empty())
        {
            if(_engine->getVerifyPeer() == 0)
            {
                if(_instance->traceLevel() >= 1)
                {
                    _instance->logger()->trace(_instance->traceCategory(),
                                               "IceSSL: ignoring certificate verification failure:\n" + trustError);
                }
            }
            else
            {
                string msg = "IceSSL: certificate verification failure:\n" + trustError;
                if(_instance->traceLevel() >= 1)
                {
                    _instance->logger()->trace(_instance->traceCategory(), msg);
                }
                throw SecurityException(__FILE__, __LINE__, msg);
            }
        }
    }

    SecPkgContext_ConnectionInfo connInfo;
    err = QueryContextAttributes(&_ssl, SECPKG_ATTR_CONNECTION_INFO, &connInfo);
    if(err == SEC_E_OK)
    {
        _cipher = _engine->getCipherName(connInfo.aiCipher);
    }
    else
    {
        throw SecurityException(__FILE__, __LINE__, "IceSSL: error reading cipher info:\n" + secStatusToString(err));
    }

    auto info = dynamic_pointer_cast<ConnectionInfo>(getInfo());
    try
    {
        _engine->verifyPeerCertName(_host, info);
    }
    catch(const Ice::SecurityException&)
    {
        _trustError = IceSSL::TrustError::HostNameMismatch;
        _verified = false;
        dynamic_pointer_cast<ExtendedConnectionInfo>(info)->errorCode = IceSSL::TrustError::HostNameMismatch;
        info->verified = false;
        if(_engine->getVerifyPeer() > 0)
        {
            throw;
        }
    }
    _engine->verifyPeer(_host, info, toString());
    _state = StateHandshakeComplete;

    if(_instance->engine()->securityTraceLevel() >= 1)
    {
        string sslCipherName;
        string sslKeyExchangeAlgorithm;
        string sslProtocolName;
        if(QueryContextAttributes(&_ssl, SECPKG_ATTR_CONNECTION_INFO, &connInfo) == SEC_E_OK)
        {
            sslCipherName = _engine->getCipherName(connInfo.aiCipher);
            sslKeyExchangeAlgorithm = _engine->getCipherName(connInfo.aiExch);
            sslProtocolName = protocolName(connInfo.dwProtocol);
        }

        Trace out(_instance->logger(), _instance->traceCategory());
        out << "SSL summary for " << (_incoming ? "incoming" : "outgoing") << " connection\n";

        if(sslCipherName.empty())
        {
            out << "unknown cipher\n";
        }
        else
        {
            out << "cipher = " << sslCipherName
                << "\nkey exchange = " << sslKeyExchangeAlgorithm
                << "\nprotocol = " << sslProtocolName << "\n";
        }
        out << toString();
    }
    _delegate->getNativeInfo()->ready(IceInternal::SocketOperationRead,
                                      !_readUnprocessed.b.empty() || _readBuffer.i != _readBuffer.b.begin());
    return IceInternal::SocketOperationNone;
}

IceInternal::SocketOperation
SChannel::TransceiverI::closing(bool initiator, exception_ptr)
{
    // If we are initiating the connection closure, wait for the peer
    // to close the TCP/IP connection. Otherwise, close immediately.
    return initiator ? IceInternal::SocketOperationRead : IceInternal::SocketOperationNone;
}

void
SChannel::TransceiverI::close()
{
    if(_sslInitialized)
    {
        DeleteSecurityContext(&_ssl);
        _sslInitialized = false;
    }

    if(_credentialsInitialized)
    {
        FreeCredentialsHandle(&_credentials);
        _credentialsInitialized = false;
    }

    _delegate->close();

    //
    // Clear the buffers now instead of waiting for destruction.
    //
    _writeBuffer.b.clear();
    _readBuffer.b.clear();
    _readUnprocessed.b.clear();
}

IceInternal::SocketOperation
SChannel::TransceiverI::write(IceInternal::Buffer& buf)
{
    if(_state == StateNotInitialized)
    {
        return _delegate->write(buf);
    }

    if(buf.i == buf.b.end())
    {
        return IceInternal::SocketOperationNone;
    }
    assert(_state == StateHandshakeComplete);

    while(buf.i != buf.b.end())
    {
        if(_bufferedW == 0)
        {
            assert(_writeBuffer.i == _writeBuffer.b.end());
            _bufferedW = encryptMessage(buf);
        }

        if(!writeRaw(_writeBuffer))
        {
            return IceInternal::SocketOperationWrite;
        }

        assert(_writeBuffer.i == _writeBuffer.b.end()); // Finished writing the encrypted data

        buf.i += _bufferedW;
        _bufferedW = 0;
    }
    return IceInternal::SocketOperationNone;
}

IceInternal::SocketOperation
SChannel::TransceiverI::read(IceInternal::Buffer& buf)
{
    if(_state == StateNotInitialized)
    {
        return _delegate->read(buf);
    }

    if(buf.i == buf.b.end())
    {
        return IceInternal::SocketOperationNone;
    }
    assert(_state == StateHandshakeComplete);

    _delegate->getNativeInfo()->ready(IceInternal::SocketOperationRead, false);
    while(buf.i != buf.b.end())
    {
        if(_readUnprocessed.b.empty() && _readBuffer.i == _readBuffer.b.begin() && !readRaw(_readBuffer))
        {
            return IceInternal::SocketOperationRead;
        }

        size_t decrypted = decryptMessage(buf);
        if(decrypted == 0)
        {
            if(!readRaw(_readBuffer))
            {
                return IceInternal::SocketOperationRead;
            }
            continue;
        }

        buf.i += decrypted;
    }
    _delegate->getNativeInfo()->ready(IceInternal::SocketOperationRead,
                                      !_readUnprocessed.b.empty() || _readBuffer.i != _readBuffer.b.begin());
    return IceInternal::SocketOperationNone;
}

#ifdef ICE_USE_IOCP

bool
SChannel::TransceiverI::startWrite(IceInternal::Buffer& buffer)
{
    if(_state == StateNotInitialized)
    {
        return _delegate->startWrite(buffer);
    }

    if(_state == StateHandshakeComplete && _bufferedW == 0)
    {
        assert(_writeBuffer.i == _writeBuffer.b.end());
        _bufferedW = encryptMessage(buffer);
    }

    return _delegate->startWrite(_writeBuffer) && _bufferedW == static_cast<size_t>((buffer.b.end() - buffer.i));
}

void
SChannel::TransceiverI::finishWrite(IceInternal::Buffer& buf)
{
    if(_state == StateNotInitialized)
    {
        _delegate->finishWrite(buf);
        return;
    }

    _delegate->finishWrite(_writeBuffer);
    if(_writeBuffer.i != _writeBuffer.b.end())
    {
        return; // We're not finished yet with writing the write buffer.
    }

    if(_state == StateHandshakeComplete)
    {
        buf.i += _bufferedW;
        _bufferedW = 0;
    }
}

void
SChannel::TransceiverI::startRead(IceInternal::Buffer& buffer)
{
    if(_state == StateNotInitialized)
    {
        _delegate->startRead(buffer);
        return;
    }
    _delegate->startRead(_readBuffer);
}

void
SChannel::TransceiverI::finishRead(IceInternal::Buffer& buf)
{
    if(_state == StateNotInitialized)
    {
        _delegate->finishRead(buf);
        return;
    }

    _delegate->finishRead(_readBuffer);
    if(_state == StateHandshakeComplete)
    {
        size_t decrypted = decryptMessage(buf);
        if(decrypted > 0)
        {
            buf.i += decrypted;
            _delegate->getNativeInfo()->ready(IceInternal::SocketOperationRead,
                                              !_readUnprocessed.b.empty() || _readBuffer.i != _readBuffer.b.begin());
        }
        else
        {
            _delegate->getNativeInfo()->ready(IceInternal::SocketOperationRead, false);
        }
    }
}
#endif

string
SChannel::TransceiverI::protocol() const
{
    return _instance->protocol();
}

string
SChannel::TransceiverI::toString() const
{
    return _delegate->toString();
}

string
SChannel::TransceiverI::toDetailedString() const
{
    return toString();
}

Ice::ConnectionInfoPtr
SChannel::TransceiverI::getInfo() const
{
    ExtendedConnectionInfoPtr info = std::make_shared<ExtendedConnectionInfo>();
    info->underlying = _delegate->getInfo();
    info->incoming = _incoming;
    info->adapterName = _adapterName;
    info->cipher = _cipher;
    info->certs = _certs;
    info->verified = _verified;
    info->errorCode = _trustError;
    info->host = _incoming ? "" : _host;
    return info;
}

void
SChannel::TransceiverI::checkSendSize(const IceInternal::Buffer&)
{
}

void
SChannel::TransceiverI::setBufferSize(int rcvSize, int sndSize)
{
    _delegate->setBufferSize(rcvSize, sndSize);
}

SChannel::TransceiverI::TransceiverI(const InstancePtr& instance,
                                   const IceInternal::TransceiverPtr& delegate,
                                   const string& hostOrAdapterName,
                                   bool incoming) :
    _instance(instance),
    _engine(dynamic_pointer_cast<SChannel::SSLEngine>(instance->engine())),
    _host(incoming ? "" : hostOrAdapterName),
    _adapterName(incoming ? hostOrAdapterName : ""),
    _incoming(incoming),
    _delegate(delegate),
    _state(StateNotInitialized),
    _bufferedW(0),
    _sslInitialized(false),
    _credentialsInitialized(false),
    _verified(false)
{
}

SChannel::TransceiverI::~TransceiverI()
{
}

bool
SChannel::TransceiverI::writeRaw(IceInternal::Buffer& buf)
{
    _delegate->write(buf);
    return buf.i == buf.b.end();
}

bool
SChannel::TransceiverI::readRaw(IceInternal::Buffer& buf)
{
    IceInternal::Buffer::Container::iterator p = buf.i;
    _delegate->read(buf);
    return buf.i != p;
}
