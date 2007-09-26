/**
 * Copyright (C) 2007 Doug Judd (Zvents, Inc.)
 * 
 * This file is part of Hypertable.
 * 
 * Hypertable is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or any later version.
 * 
 * Hypertable is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include <cassert>

extern "C" {
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
}

#include "AsyncComm/DispatchHandlerSynchronizer.h"
#include "AsyncComm/Comm.h"
#include "AsyncComm/ConnectionManager.h"
#include "AsyncComm/Serialization.h"

#include "Common/Error.h"
#include "Common/InetAddr.h"
#include "Common/Logger.h"
#include "Common/Properties.h"

#include "ClientHandleState.h"
#include "Master.h"
#include "Protocol.h"
#include "Session.h"

using namespace hypertable;
using namespace Hyperspace;

const uint32_t Session::DEFAULT_CLIENT_PORT;

/**
 * 
 */
Session::Session(Comm *comm, PropertiesPtr &propsPtr, SessionCallback *callback) : mComm(comm), mVerbose(false), mState(STATE_JEOPARDY), mSessionCallback(callback) {
  uint16_t masterPort;
  const char *masterHost;

  mVerbose = propsPtr->getPropertyBool("verbose", false);
  masterHost = propsPtr->getProperty("Hyperspace.Master.host", "localhost");
  masterPort = (uint16_t)propsPtr->getPropertyInt("Hyperspace.Master.port", Master::DEFAULT_MASTER_PORT);
  mGracePeriod = (uint32_t)propsPtr->getPropertyInt("Hyperspace.GracePeriod", Master::DEFAULT_GRACEPERIOD);

  if (!InetAddr::Initialize(&mMasterAddr, masterHost, masterPort))
    exit(1);

  boost::xtime_get(&mExpireTime, boost::TIME_UTC);
  mExpireTime.sec += mGracePeriod;

  if (mVerbose) {
    cout << "Hyperspace.GracePeriod=" << mGracePeriod << endl;
  }

  mKeepaliveHandler = new ClientKeepaliveHandler(comm, propsPtr, this);
}


int Session::Open(std::string name, uint32_t flags, HandleCallbackPtr &callbackPtr, uint64_t *handlep, bool *createdp) {
  DispatchHandlerSynchronizer syncHandler;
  hypertable::EventPtr eventPtr;
  CommBufPtr cbufPtr( Protocol::CreateOpenRequest(name, flags, callbackPtr) );
  ClientHandleStatePtr handleStatePtr( new ClientHandleState() );

  handleStatePtr->handle = 0;
  handleStatePtr->openFlags = flags;
  handleStatePtr->callbackPtr = callbackPtr;
  NormalizeName(name, handleStatePtr->normalName);
  handleStatePtr->sequencer = 0;
  handleStatePtr->lockStatus = 0;

 try_again:
  if (!WaitForSafe())
    return Error::HYPERSPACE_EXPIRED_SESSION;
  
  int error = SendMessage(cbufPtr, &syncHandler);
  if (error == Error::OK) {
    if (!syncHandler.WaitForReply(eventPtr)) {
      LOG_VA_ERROR("Hyperspace 'open' error, name=%s flags=0x%x events=0x%x : %s", name.c_str(), 
		   flags, callbackPtr->GetEventMask(), Protocol::StringFormatMessage(eventPtr.get()).c_str());
      error = (int)Protocol::ResponseCode(eventPtr.get());
    }
    else {
      uint8_t *ptr = eventPtr->message + 4;
      size_t remaining = eventPtr->messageLen - 4;
      uint8_t cbyte;
      if (!Serialization::DecodeLong(&ptr, &remaining, handlep))
	return Error::RESPONSE_TRUNCATED;
      if (!Serialization::DecodeByte(&ptr, &remaining, &cbyte))
	return Error::RESPONSE_TRUNCATED;
      *createdp = cbyte ? true : false;
      handleStatePtr->handle = *handlep;
      mKeepaliveHandler->RegisterHandle(handleStatePtr);
    }
  }
  else {
    StateTransition(Session::STATE_JEOPARDY);
    goto try_again;
  }

  return error;
}


/**
 *
 */
int Session::Close(uint64_t handle) {
  DispatchHandlerSynchronizer syncHandler;
  hypertable::EventPtr eventPtr;
  CommBufPtr cbufPtr( Protocol::CreateCloseRequest(handle) );

 try_again:
  if (!WaitForSafe())
    return Error::HYPERSPACE_EXPIRED_SESSION;

  int error = SendMessage(cbufPtr, &syncHandler);
  if (error == Error::OK) {
    if (!syncHandler.WaitForReply(eventPtr)) {
      LOG_VA_ERROR("Hyperspace 'close' error : %s", Protocol::StringFormatMessage(eventPtr.get()).c_str());
      error = (int)Protocol::ResponseCode(eventPtr.get());
    }
  }
  else {
    StateTransition(Session::STATE_JEOPARDY);
    goto try_again;
  }

  return error;
}



/**
 * Submit 'mkdir' request
 */
int Session::Mkdir(std::string name) {
  DispatchHandlerSynchronizer syncHandler;
  hypertable::EventPtr eventPtr;
  CommBufPtr cbufPtr( Protocol::CreateMkdirRequest(name) );

 try_again:
  if (!WaitForSafe())
    return Error::HYPERSPACE_EXPIRED_SESSION;
  
  int error = SendMessage(cbufPtr, &syncHandler);
  if (error == Error::OK) {
    if (!syncHandler.WaitForReply(eventPtr)) {
      LOG_VA_ERROR("Hyperspace 'mkdir' error, name=%s : %s", name.c_str(), Protocol::StringFormatMessage(eventPtr.get()).c_str());
      error = (int)Protocol::ResponseCode(eventPtr.get());
    }
  }
  else {
    StateTransition(Session::STATE_JEOPARDY);
    goto try_again;
  }

  return error;
}


int Session::Delete(std::string name) {
  DispatchHandlerSynchronizer syncHandler;
  hypertable::EventPtr eventPtr;
  CommBufPtr cbufPtr( Protocol::CreateDeleteRequest(name) );

 try_again:
  if (!WaitForSafe())
    return Error::HYPERSPACE_EXPIRED_SESSION;
  
  int error = SendMessage(cbufPtr, &syncHandler);
  if (error == Error::OK) {
    if (!syncHandler.WaitForReply(eventPtr)) {
      LOG_VA_ERROR("Hyperspace 'delete' error, name=%s : %s", name.c_str(), Protocol::StringFormatMessage(eventPtr.get()).c_str());
      error = (int)Protocol::ResponseCode(eventPtr.get());
    }
  }
  else {
    StateTransition(Session::STATE_JEOPARDY);
    goto try_again;
  }

  return error;
}


int Session::Exists(std::string name, bool *existsp) {
  DispatchHandlerSynchronizer syncHandler;
  hypertable::EventPtr eventPtr;
  CommBufPtr cbufPtr( Protocol::CreateExistsRequest(name) );

 try_again:
  if (!WaitForSafe())
    return Error::HYPERSPACE_EXPIRED_SESSION;
  
  int error = SendMessage(cbufPtr, &syncHandler);
  if (error == Error::OK) {
    if (!syncHandler.WaitForReply(eventPtr)) {
      LOG_VA_ERROR("Hyperspace 'exists' error, name=%s : %s", name.c_str(), Protocol::StringFormatMessage(eventPtr.get()).c_str());
      error = (int)Protocol::ResponseCode(eventPtr.get());
    }
    else {
      uint8_t *ptr = eventPtr->message + 4;
      size_t remaining = eventPtr->messageLen - 4;
      uint8_t bval;
      if (!Serialization::DecodeByte(&ptr, &remaining, &bval))
	assert(!"problem decoding return packet");
      *existsp = (bval == 0) ? false : true;
    }
  }
  else {
    StateTransition(Session::STATE_JEOPARDY);
    goto try_again;
  }

  return error;
}



/**
 *  Blocking 'attrset' request
 */
int Session::AttrSet(uint64_t handle, std::string name, const void *value, size_t valueLen) {
  DispatchHandlerSynchronizer syncHandler;
  hypertable::EventPtr eventPtr;
  CommBufPtr cbufPtr( Protocol::CreateAttrSetRequest(handle, name, value, valueLen) );

 try_again:
  if (!WaitForSafe())
    return Error::HYPERSPACE_EXPIRED_SESSION;

  int error = SendMessage(cbufPtr, &syncHandler);
  if (error == Error::OK) {
    if (!syncHandler.WaitForReply(eventPtr)) {
      LOG_VA_ERROR("Hyperspace 'attrset' error, name=%s : %s", name.c_str(), Protocol::StringFormatMessage(eventPtr.get()).c_str());
      error = (int)Protocol::ResponseCode(eventPtr.get());
    }
  }
  else {
    StateTransition(Session::STATE_JEOPARDY);
    goto try_again;
  }

  return error;
}


/**
 *
 */
int Session::AttrGet(uint64_t handle, std::string name, DynamicBuffer &value) {
  DispatchHandlerSynchronizer syncHandler;
  hypertable::EventPtr eventPtr;
  CommBufPtr cbufPtr( Protocol::CreateAttrGetRequest(handle, name) );

 try_again:
  if (!WaitForSafe())
    return Error::HYPERSPACE_EXPIRED_SESSION;

  int error = SendMessage(cbufPtr, &syncHandler);
  if (error == Error::OK) {
    if (!syncHandler.WaitForReply(eventPtr)) {
      LOG_VA_ERROR("Hyperspace 'attrget' error, name=%s : %s", name.c_str(), Protocol::StringFormatMessage(eventPtr.get()).c_str());
      error = (int)Protocol::ResponseCode(eventPtr.get());
    }
    else {
      uint8_t *attrValue;
      int32_t attrValueLen;
      uint8_t *ptr = eventPtr->message + 4;
      size_t remaining = eventPtr->messageLen - 4;
      if (!Serialization::DecodeByteArray(&ptr, &remaining, &attrValue, &attrValueLen)) {
	assert(!"problem decoding return packet");
      }
      value.clear();
      value.add(attrValue, attrValueLen);
    }
  }
  else {
    StateTransition(Session::STATE_JEOPARDY);
    goto try_again;
  }

  return error;
}


/**
 *
 */
int Session::AttrDel(uint64_t handle, std::string name) {
  DispatchHandlerSynchronizer syncHandler;
  hypertable::EventPtr eventPtr;
  CommBufPtr cbufPtr( Protocol::CreateAttrDelRequest(handle, name) );

 try_again:
  if (!WaitForSafe())
    return Error::HYPERSPACE_EXPIRED_SESSION;

  int error = SendMessage(cbufPtr, &syncHandler);
  if (error == Error::OK) {
    if (!syncHandler.WaitForReply(eventPtr)) {
      LOG_VA_ERROR("Hyperspace 'attrdel' error, name=%s : %s", name.c_str(), Protocol::StringFormatMessage(eventPtr.get()).c_str());
      error = (int)Protocol::ResponseCode(eventPtr.get());
    }
  }
  else {
    StateTransition(Session::STATE_JEOPARDY);
    goto try_again;
  }

  return error;
}


int Session::Lock(uint64_t handle, uint32_t mode, struct LockSequencerT *sequencerp) {
  DispatchHandlerSynchronizer syncHandler;
  hypertable::EventPtr eventPtr;
  CommBufPtr cbufPtr( Protocol::CreateLockRequest(handle, mode, false) );
  ClientHandleStatePtr handleStatePtr;

  if (!mKeepaliveHandler->GetHandleState(handle, handleStatePtr))
    return Error::HYPERSPACE_INVALID_HANDLE;

 try_again:
  if (!WaitForSafe())
    return Error::HYPERSPACE_EXPIRED_SESSION;

  int error = SendMessage(cbufPtr, &syncHandler);
  if (error == Error::OK) {
    if (!syncHandler.WaitForReply(eventPtr)) {
      LOG_VA_ERROR("Hyperspace 'lock' error, handle=%lld name='%s': %s",
		   handle, handleStatePtr->normalName.c_str(), Protocol::StringFormatMessage(eventPtr.get()).c_str());
      error = (int)Protocol::ResponseCode(eventPtr.get());
    }
    else {
      // TBD
    }
  }
  else {
    StateTransition(Session::STATE_JEOPARDY);
    goto try_again;
  }

  return error;
}



int Session::TryLock(uint64_t handle, uint32_t mode, uint32_t *statusp, struct LockSequencerT *sequencerp) {
  DispatchHandlerSynchronizer syncHandler;
  hypertable::EventPtr eventPtr;
  CommBufPtr cbufPtr( Protocol::CreateLockRequest(handle, mode, true) );
  ClientHandleStatePtr handleStatePtr;

  if (!mKeepaliveHandler->GetHandleState(handle, handleStatePtr))
    return Error::HYPERSPACE_INVALID_HANDLE;

 try_again:
  if (!WaitForSafe())
    return Error::HYPERSPACE_EXPIRED_SESSION;

  int error = SendMessage(cbufPtr, &syncHandler);
  if (error == Error::OK) {
    if (!syncHandler.WaitForReply(eventPtr)) {
      LOG_VA_ERROR("Hyperspace 'trylock' error, handle=%lld name='%s': %s",
		   handle, handleStatePtr->normalName.c_str(), Protocol::StringFormatMessage(eventPtr.get()).c_str());
      error = (int)Protocol::ResponseCode(eventPtr.get());
    }
    else {
      uint8_t *ptr = eventPtr->message + 4;
      size_t remaining = eventPtr->messageLen - 4;
      if (!Serialization::DecodeInt(&ptr, &remaining, statusp))
	assert(!"problem decoding return packet");
      if (*statusp == LOCK_STATUS_GRANTED) {
	if (!Serialization::DecodeLong(&ptr, &remaining, &sequencerp->generation))
	  assert(!"problem decoding return packet");
	sequencerp->mode = mode;
	sequencerp->name = handleStatePtr->normalName;
      }
    }
  }
  else {
    StateTransition(Session::STATE_JEOPARDY);
    goto try_again;
  }

  return error;
}




/**
 * Transition session state
 */
int Session::StateTransition(int state) {
  boost::mutex::scoped_lock lock(mMutex);
  int oldState = mState;
  mState = state;
  if (mState == STATE_SAFE) {
    mCond.notify_all();
    if (oldState == STATE_JEOPARDY)
      mSessionCallback->Safe();
  }
  else if (mState == STATE_JEOPARDY) {
    if (oldState == STATE_SAFE) {
      mSessionCallback->Jeopardy();
      boost::xtime_get(&mExpireTime, boost::TIME_UTC);
      mExpireTime.sec += mGracePeriod;
    }
  }
  else if (mState == STATE_EXPIRED) {
    if (oldState != STATE_EXPIRED)
      mSessionCallback->Expired();
    mCond.notify_all();
  }
  return oldState;
}



/**
 * Return Sesion state
 */
int Session::GetState() {
  boost::mutex::scoped_lock lock(mMutex);
  return mState;
}


/**
 * Return true if session expired
 */
bool Session::Expired() {
  boost::mutex::scoped_lock lock(mMutex);
  boost::xtime now;
  boost::xtime_get(&now, boost::TIME_UTC);
  if (xtime_cmp(mExpireTime, now) < 0)
    return true;
  return false;
}


/**
 * Waits for session state to change to SAFE
 */
bool Session::WaitForConnection(long maxWaitSecs) {
  boost::mutex::scoped_lock lock(mMutex);
  boost::xtime dropTime, now;

  boost::xtime_get(&dropTime, boost::TIME_UTC);
  dropTime.sec += maxWaitSecs;

  while (mState != STATE_SAFE) {
    mCond.timed_wait(lock, dropTime);
    boost::xtime_get(&now, boost::TIME_UTC);
    if (xtime_cmp(now, dropTime) >= 0)
      return false;
  }
  return true;
}



bool Session::WaitForSafe() {
  boost::mutex::scoped_lock lock(mMutex);
  while (mState != STATE_SAFE) {
    if (mState == STATE_EXPIRED)
      return false;
    mCond.wait(lock);
  }
  return true;
}

int Session::SendMessage(CommBufPtr &cbufPtr, DispatchHandler *handler) {
  int error;

  if ((error = mComm->SendRequest(mMasterAddr, cbufPtr, handler)) != Error::OK) {
    std::string str;
    LOG_VA_WARN("Comm::SendRequest to Hypertable.Master at %s failed - %s",
		InetAddr::StringFormat(str, mMasterAddr), Error::GetText(error));
  }
  return error;
}


/**
 *
 */
void Session::NormalizeName(std::string name, std::string &normal) {
  normal = "";
  if (name[0] != '/')
    normal += "/";

  if (name.find('/', name.length()-1) == string::npos)
    normal += name;
  else
    normal += name.substr(0, name.length()-1);
}






#if 0

/**
 * Blocking 'attrget' method
 */
int Session::AttrGet(const char *fname, const char *aname, DynamicBuffer &out) {
  DispatchHandlerSynchronizer syncHandler;
  hypertable::EventPtr eventPtr;
  CommBufPtr cbufPtr( mProtocol->CreateAttrGetRequest(fname, aname) );
  out.clear();
  int error = SendMessage(cbufPtr, &syncHandler);
  if (error == Error::OK) {
    if (!syncHandler.WaitForReply(eventPtr)) {
      LOG_VA_WARN("Hyperspace 'attrget' error, fname=%s aname=%s : %s", fname, aname, mProtocol->StringFormatMessage(eventPtr).c_str());
      error = (int)mProtocol->ResponseCode(eventPtr);
    }
    else {
      if (eventPtr->messageLen < 7) {
	LOG_VA_ERROR("Hyperspace 'attrget' error, fname=%s aname=%s : short response", fname, aname);
	error = Error::PROTOCOL_ERROR;
      }
      else {
	uint8_t *ptr = eventPtr->message + 4;
	size_t remaining = eventPtr->messageLen - 4;
	if (!Serialization::DecodeString(&ptr, &remaining, &avalue))
	  assert(!"problem decoding return packet");
	if (*avalue != 0) {
	  out.reserve(strlen(avalue)+1);
	  out.addNoCheck(avalue, strlen(avalue));
	  *out.ptr = 0;
	}
      }
    }
  }
  return error;
}

#endif
