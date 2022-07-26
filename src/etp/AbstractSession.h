/*-----------------------------------------------------------------------
Licensed to the Apache Software Foundation (ASF) under one
or more contributor license agreements.  See the NOTICE file
distributed with this work for additional information
regarding copyright ownership.  The ASF licenses this file
to you under the Apache License, Version 2.0 (the
"License"; you may not use this file except in compliance
with the License.  You may obtain a copy of the License at

  http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing,
software distributed under the License is distributed on an
"AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
KIND, either express or implied.  See the License for the
specific language governing permissions and limitations
under the License.
-----------------------------------------------------------------------*/
#pragma once

#include <boost/beast/core.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/uuid/uuid_io.hpp>

#include "../nsDefinitions.h"

#include "EtpHelpers.h"
#include "ProtocolHandlers/CoreHandlers.h"
#include "ProtocolHandlers/DiscoveryHandlers.h"
#include "ProtocolHandlers/StoreHandlers.h"
#include "ProtocolHandlers/StoreNotificationHandlers.h"
#include "ProtocolHandlers/DataArrayHandlers.h"
#include "ProtocolHandlers/TransactionHandlers.h"
#include "ProtocolHandlers/DataspaceHandlers.h"

#include <unordered_map>
#include <queue>

using tcp = boost::asio::ip::tcp;               // from <boost/asio/ip/tcp.hpp>
namespace websocket = boost::beast::websocket;  // from <boost/beast/websocket.hpp>

namespace {
	std::vector<std::string> tokenize(const std::string & str, char delimiter) {
		std::vector<std::string> tokens;
		std::stringstream ss(str);
		std::string token;
		while(getline(ss, token, delimiter)) {
			tokens.push_back(token);
		}

		return tokens;
	}
}

namespace ETP_NS
{
	class AbstractSession : public std::enable_shared_from_this<AbstractSession>
	{
	public:

		virtual ~AbstractSession() = default;

		/**
		* Return the identifier of the session which is an UUID.
		*/
		const boost::uuids::uuid& getIdentifier() const { return identifier; }

		/**
		* The list of subscriptions recorded by customers on this session.
		*/
		std::unordered_map<int64_t, Energistics::Etp::v12::Datatypes::Object::SubscriptionInfo> subscriptions;

		virtual boost::asio::io_context& getIoContext() = 0;

		void flushReceivingBuffer() {
			receivedBuffer.consume(receivedBuffer.size());
		}

		/**
		 * Set the Core protocol handlers
		 */
		FETPAPI_DLL_IMPORT_OR_EXPORT void setCoreProtocolHandlers(std::shared_ptr<CoreHandlers> coreHandlers) {
	    	if (protocolHandlers.empty()) {
	    		protocolHandlers.push_back(coreHandlers);
	    	}
	    	else {
	    		protocolHandlers[static_cast<int32_t>(Energistics::Etp::v12::Datatypes::Protocol::Core)] = coreHandlers;
	    	}
	    }

	    /**
		 * Set the Discovery protocol handlers
		 */
		FETPAPI_DLL_IMPORT_OR_EXPORT void setDiscoveryProtocolHandlers(std::shared_ptr<DiscoveryHandlers> discoveryHandlers) {
			while (protocolHandlers.size() < static_cast<int32_t>(Energistics::Etp::v12::Datatypes::Protocol::Discovery) + 1) {
				protocolHandlers.push_back(nullptr);
			}
			protocolHandlers[static_cast<int32_t>(Energistics::Etp::v12::Datatypes::Protocol::Discovery)] = discoveryHandlers;
		}

		FETPAPI_DLL_IMPORT_OR_EXPORT std::shared_ptr<ETP_NS::ProtocolHandlers> getDiscoveryProtocolHandlers() {
			return protocolHandlers.size() > static_cast<int32_t>(Energistics::Etp::v12::Datatypes::Protocol::Discovery) ?
				protocolHandlers[static_cast<int32_t>(Energistics::Etp::v12::Datatypes::Protocol::Discovery)] :
				nullptr;
		}

		/**
		 * Set the Store protocol handlers
		 */
		FETPAPI_DLL_IMPORT_OR_EXPORT void setStoreProtocolHandlers(std::shared_ptr<StoreHandlers> storeHandlers) {
			while (protocolHandlers.size() < static_cast<int32_t>(Energistics::Etp::v12::Datatypes::Protocol::Store) + 1) {
				protocolHandlers.push_back(nullptr);
			}
			protocolHandlers[static_cast<int32_t>(Energistics::Etp::v12::Datatypes::Protocol::Store)] = storeHandlers;
		}

		FETPAPI_DLL_IMPORT_OR_EXPORT std::shared_ptr<ETP_NS::ProtocolHandlers> getStoreProtocolHandlers() {
			return protocolHandlers.size() > static_cast<int32_t>(Energistics::Etp::v12::Datatypes::Protocol::Store) ?
				protocolHandlers[static_cast<int32_t>(Energistics::Etp::v12::Datatypes::Protocol::Store)] :
				nullptr;
		}

		/**
		* Set the StoreNotification protocol handlers
		*/
		FETPAPI_DLL_IMPORT_OR_EXPORT void setStoreNotificationProtocolHandlers(std::shared_ptr<ETP_NS::StoreNotificationHandlers> storeNotificationHandlers) {
			while (protocolHandlers.size() < static_cast<int32_t>(Energistics::Etp::v12::Datatypes::Protocol::StoreNotification) + 1) {
				protocolHandlers.push_back(nullptr);
			}
			protocolHandlers[static_cast<int32_t>(Energistics::Etp::v12::Datatypes::Protocol::StoreNotification)] = storeNotificationHandlers;
		}

		FETPAPI_DLL_IMPORT_OR_EXPORT std::shared_ptr<ETP_NS::ProtocolHandlers> getStoreNotificationProtocolHandlers() {
			return protocolHandlers.size() > static_cast<int32_t>(Energistics::Etp::v12::Datatypes::Protocol::StoreNotification) ?
				protocolHandlers[static_cast<int32_t>(Energistics::Etp::v12::Datatypes::Protocol::StoreNotification)] :
				nullptr;
		}

		/**
		 * Set the Data Array protocol handlers
		 */
		FETPAPI_DLL_IMPORT_OR_EXPORT void setDataArrayProtocolHandlers(std::shared_ptr<DataArrayHandlers> dataArrayHandlers) {
			while (protocolHandlers.size() < static_cast<int32_t>(Energistics::Etp::v12::Datatypes::Protocol::DataArray) + 1) {
				protocolHandlers.push_back(nullptr);
			}
			protocolHandlers[static_cast<int32_t>(Energistics::Etp::v12::Datatypes::Protocol::DataArray)] = dataArrayHandlers;
		}

		/**
		 * Set the Transaction protocol handlers
		 */
		FETPAPI_DLL_IMPORT_OR_EXPORT void setTransactionProtocolHandlers(std::shared_ptr<TransactionHandlers> transactionHandlers) {
			while (protocolHandlers.size() < static_cast<int32_t>(Energistics::Etp::v12::Datatypes::Protocol::Transaction) + 1) {
				protocolHandlers.push_back(nullptr);
			}
			protocolHandlers[static_cast<int32_t>(Energistics::Etp::v12::Datatypes::Protocol::Transaction)] = transactionHandlers;
		}

		/**
		 * Set the Dataspace protocol handlers
		 */
		FETPAPI_DLL_IMPORT_OR_EXPORT void setDataspaceProtocolHandlers(std::shared_ptr<DataspaceHandlers> dataspaceHandlers) {
			while (protocolHandlers.size() < static_cast<int32_t>(Energistics::Etp::v12::Datatypes::Protocol::Dataspace) + 1) {
				protocolHandlers.push_back(nullptr);
			}
			protocolHandlers[static_cast<int32_t>(Energistics::Etp::v12::Datatypes::Protocol::Dataspace)] = dataspaceHandlers;
		}

		/**
		* Wait for all sent messages to be responded and then close the websocket connection
		*/
		FETPAPI_DLL_IMPORT_OR_EXPORT void close() { 
			isCloseRequested = true;
			if (specificProtocolHandlers.empty() && sendingQueue.empty()) {
				etpSessionClosed = true;
				send(Energistics::Etp::v12::Protocol::Core::CloseSession(), 0, 0x02);
			}
		}

		/**
		 * Create a default ETP message header from the ETP message body.
		 * Encode this created default ETP message header + the ETP message body and put the result in the sending queue.
		 *
		 * @param mb The ETP message body to send
		 * @param correlationId The ID of the message which this message is answering to.
		 * @param messageFlags The message flags to be sent within the header
		 * @return The ID of the message that has been put in the sending queue.
		 */
		template<typename T> int64_t send(const T & mb, int64_t correlationId = 0, int32_t messageFlags = 0)
		{
			if (protocolHandlers.size() > mb.protocolId) {
				return sendWithSpecificHandler(mb, protocolHandlers[mb.protocolId], correlationId, messageFlags);
			}
			else {
				throw std::logic_error("The agent has no registered handler at all for the protocol " + std::to_string(mb.protocolId));
			}
		}

		/**
		* Send a message to the server and block the thread until the answer of the server has been processed by the handlers
		*
		* @param mb The ETP message body to send
		* @param correlationId The ID of the message which this message is answering to.
		* @param messageFlags The message flags to be sent within the header
		* @return The ID of the message that has been put in the sending queue.
		*/
		template<typename T> void sendAndBlock(const T & mb, int64_t correlationId = 0, int32_t messageFlags = 0)
		{
			int64_t msgId = send(mb, correlationId, messageFlags);
			while (isMessageStillProcessing(msgId)) {}
		}

		/**
		* Send a message and register a specific handler for the response.
		*
		* @param mb The ETP message body to send
		* @param specificHandler The handler which are going to be called for the response to this sent message
		* @param correlationId The ID of the message which this message is answering to.
		* @param messageFlags The message flags to be sent within the header
		* @return The ID of the message that has been put in the sending queue.
		*/
		template<typename T> int64_t sendWithSpecificHandler(const T & mb, std::shared_ptr<ETP_NS::ProtocolHandlers> specificHandler, int64_t correlationId = 0, int32_t messageFlags = 0)
		{
			// Encode the message into AVRO format and put it in the sending queue
			const int64_t msgId = encode(mb, correlationId, messageFlags);
			std::get<2>(sendingQueue.back()) = specificHandler;

			// Send the message directly if the sending queue was empty.
			if (sendingQueue.size() == 1) {
				do_write();
			}

			return msgId;
		}

		/**
		 * Close the web socket session (without sending any ETP message)
		 */
		virtual void do_close() = 0;

		/**
		 * Read what is on the websocket
		 */
		virtual void do_read() = 0;

		/**
		 * This method is called each time a message is received on the web socket
		 */
		FETPAPI_DLL_IMPORT_OR_EXPORT void on_read(boost::system::error_code ec, std::size_t bytes_transferred);

		void on_write(boost::system::error_code ec, std::size_t) {
			if(ec) {
				std::cerr << "on_write : " << ec.message() << std::endl;
			}

			// Remove the sent message from the queue
			sendingQueue.pop();

			do_write();
		}

		void on_close(boost::system::error_code ec) {
			if(ec) {
				std::cerr << "on_close : " << ec.message() << std::endl;
			}
			if (!etpSessionClosed) {
				std::cerr << "Websocket session is going to be closed BUT ETP SESSION HAS NOT BEEN CLOSED YET!!!" << std::endl;
			}

			// If we get here then the connection is closed gracefully
			webSocketSessionClosed = true;
		}

		/**
		* Check if the websocket session (starting after the HTTP handshake/upgrade) is not opened yet or has been closed.
		*/
		FETPAPI_DLL_IMPORT_OR_EXPORT bool isWebSocketSessionClosed() const { return webSocketSessionClosed;  }

		FETPAPI_DLL_IMPORT_OR_EXPORT void setEtpSessionClosed(bool etpSessionClosed_) { etpSessionClosed = etpSessionClosed_; }

		/**
		* Check if the ETP session (starting after the Core.OpenSession or Core.RequestSession message) is not opened yet or has been closed.
		*/
		FETPAPI_DLL_IMPORT_OR_EXPORT bool isEtpSessionClosed() const { return webSocketSessionClosed || etpSessionClosed; }

		/**
		* Check wether a particular ETP message has been responded or not by the other agent.
		*/
		FETPAPI_DLL_IMPORT_OR_EXPORT bool isMessageStillProcessing(int64_t msgId) const { return specificProtocolHandlers.count(msgId) > 0; }

		virtual void setMaxWebSocketMessagePayloadSize(int64_t value) = 0;
		int64_t getMaxWebSocketMessagePayloadSize() const { return maxWebSocketMessagePayloadSize; }

		void setMaxSentAndNonRespondedMessageCount(uint8_t value) { maxSentAndNonRespondedMessageCount = value; }

	protected:
		boost::beast::flat_buffer receivedBuffer;
		/// The default handlers for each subprotocole. Default handlers are at the index of the corresponding subprotocol id.
		std::vector<std::shared_ptr<ETP_NS::ProtocolHandlers>> protocolHandlers;
		/// A map indicating which handlers must be used for responding to which message id.
		std::unordered_map<int64_t, std::shared_ptr<ETP_NS::ProtocolHandlers>> specificProtocolHandlers;
		/// The maximum size in bytes allowed for a complete WebSocket message payload, which is composed of one or more WebSocket frames.
		/// The limit to use during a session is the smaller of the client's and the server's value for MaxWebSocketMessagePayloadSize,
		/// which should be determined by the limits imposed by the WebSocket library used by each endpoint. 
		/// See https://www.boost.org/doc/libs/1_75_0/libs/beast/doc/html/beast/using_websocket/messages.html
		/// and https://www.boost.org/doc/libs/1_75_0/libs/beast/doc/html/beast/ref/boost__beast__websocket__stream/read_message_max/overload1.html
		int64_t maxWebSocketMessagePayloadSize = 16000000;
		/// The maximum count of messaeg which have been sent and have not been responded yet.
		uint8_t maxSentAndNonRespondedMessageCount = 5;
		/// Indicates if the websocket session is opened or not. It becomes false after the websocket handshake
		bool webSocketSessionClosed = true;
		/// Indicates if the ETP1.2 session is opened or not. It becomes false after the requestSession and openSession message
		bool etpSessionClosed = true;
		/// The queue of messages to be sent where the tuple respectively define message id, message and protoocol handlers for responding to this message.
		std::queue< std::tuple<int64_t, std::vector<uint8_t>, std::shared_ptr<ETP_NS::ProtocolHandlers>> > sendingQueue;
		/// The next available message id.
		int64_t messageId;
		/// The identifier of the session
		boost::uuids::uuid identifier;
		/// Indicates that the endpoint request to close the websocket session 
		bool isCloseRequested = false;

		AbstractSession() = default;

		/**
		 * Write the current buffer on the web socket
		 */
		virtual void do_write() = 0;

		/**
		 * Reads the message header currently stored in the decoder.
		 * @param decoder	Must be initialized with stream containing a coded message header.
		 */
		Energistics::Etp::v12::Datatypes::MessageHeader decodeMessageHeader(avro::DecoderPtr decoder);

		/**
		* Return the current message id and increment it for next call.
		*/
		int64_t incrementMessageId() {
			const int64_t result = messageId;
			messageId += 2;
			return  result;
		}

		template<typename T> int64_t encode(const T & mb, int64_t correlationId = 0, int32_t messageFlags = 0)
		{
			// Build message header
			Energistics::Etp::v12::Datatypes::MessageHeader mh;
			mh.protocol = mb.protocolId;
			mh.messageType = mb.messageTypeId;
			mh.correlationId = correlationId;
			mh.messageId = incrementMessageId();
			mh.messageFlags = messageFlags;

			avro::OutputStreamPtr out = avro::memoryOutputStream();
			avro::EncoderPtr e = avro::binaryEncoder();
			e->init(*out);
			avro::encode(*e, mh);
			avro::encode(*e, mb);
			e->flush();
			const int64_t byteCount = e->byteCount();

			if (byteCount < maxWebSocketMessagePayloadSize) {
				sendingQueue.push(std::make_tuple(mh.messageId, *avro::snapshot(*out).get(), nullptr));

				std::cout << "*************************************************" << std::endl;
				std::cout << "Message Header sent in the queue : " << std::endl;
				std::cout << "protocol : " << mh.protocol << std::endl;
				std::cout << "type : " << mh.messageType << std::endl;
				std::cout << "id : " << mh.messageId << std::endl;
				std::cout << "correlation id : " << mh.correlationId << std::endl;
				std::cout << "flags : " << mh.messageFlags << std::endl;
				std::cout << "Whole message size : " << e->byteCount() << " bytes." << std::endl;
				std::cout << "*************************************************" << std::endl;
			}
			else {
				messageId -= 2;
				if (correlationId != 0) {
					encode(EtpHelpers::buildSingleMessageProtocolException(17, "I try to send you a too big message response of protocol "
						+ std::to_string(mb.protocolId) + " and type id " + std::to_string(mb.messageTypeId) + " and size " + std::to_string(byteCount)
						+ " bytes according to our negotiated size capability which is " + std::to_string(maxWebSocketMessagePayloadSize) + " bytes."), correlationId, 0x02);
				}
				else {
					return -1; // You cannot send a message which is too big. Please use message part or chunk or whatever else.
				}
			}

			return mh.messageId;
		}
	};
}
