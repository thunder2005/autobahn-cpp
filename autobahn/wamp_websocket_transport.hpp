///////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) Tavendo GmbH and contributors.
//
// Boost Software License - Version 1.0 - August 17th, 2003
//
// Permission is hereby granted, free of charge, to any person or organization
// obtaining a copy of the software and accompanying documentation covered by
// this license (the "Software") to use, reproduce, display, distribute,
// execute, and transmit the Software, and to prepare derivative works of the
// Software, and to permit third-parties to whom the Software is furnished to
// do so, all subject to the following:
//
// The copyright notices in the Software and this entire statement, including
// the above license grant, this restriction and the following disclaimer,
// must be included in all copies of the Software, in whole or in part, and
// all derivative works of the Software, unless such copies or derivative
// works are solely in the form of machine-executable object code generated by
// a source language processor.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO EVENT
// SHALL THE COPYRIGHT HOLDERS OR ANYONE DISTRIBUTING THE SOFTWARE BE LIABLE
// FOR ANY DAMAGES OR OTHER LIABILITY, WHETHER IN CONTRACT, TORT OR OTHERWISE,
// ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
// DEALINGS IN THE SOFTWARE.
//
///////////////////////////////////////////////////////////////////////////////

#ifndef AUTOBAHN_WEBSOCKET_TRANSPORT_HPP
#define AUTOBAHN_WEBSOCKET_TRANSPORT_HPP

// http://stackoverflow.com/questions/22597948/using-boostfuture-with-then-continuations/
#define BOOST_THREAD_PROVIDES_FUTURE
#define BOOST_THREAD_PROVIDES_FUTURE_CONTINUATION
#define BOOST_THREAD_PROVIDES_FUTURE_WHEN_ALL_WHEN_ANY

#include "wamp_transport.hpp"


#include <boost/thread/future.hpp>
#include <boost/asio/io_service.hpp>
#include <cstddef>
#include <memory>
#include <msgpack.hpp>

namespace autobahn {

    class wamp_message;
    class wamp_transport_handler;

    /*!
    * A class that represents a base websocket transport
    *
    * Unlike raw socket base class, there is nothing to share about underlying websocket,
    * It is completely up to derived class to handle specific websocket implementation
    */
    class wamp_websocket_transport :
        public wamp_transport,
        public std::enable_shared_from_this<wamp_websocket_transport>
    {
    public:
        /*!
        * Constructs a websocket transport.
        *
        * @param uri The remote endpoint to connect to.
        */
        wamp_websocket_transport(
            const std::string& uri,
            bool debug_enabled = false);

        virtual ~wamp_websocket_transport() override = default;


        /*
        * CONNECTION INTERFACE
        */
        /*!
        * @copydoc wamp_transport::connect()
        */
        virtual boost::future<void> connect() override;

        /*!
        * @copydoc wamp_transport::disconnect()
        */
        virtual boost::future<void> disconnect() override;

        /*!
        * @copydoc wamp_transport::is_connected()
        */
        virtual bool is_connected() const override;

        /*
        * SENDER INTERFACE
        */
        /*!
        * @copydoc wamp_transport::send_message()
        */
        virtual void send_message(wamp_message&& message) override;

        /*!
        * @copydoc wamp_transport::set_pause_handler()
        */
        virtual void set_pause_handler(pause_handler&& handler) override;

        /*!
        * @copydoc wamp_transport::set_resume_handler()
        */
        virtual void set_resume_handler(resume_handler&& handler) override;

        /*
        * RECEIVER INTERFACE
        */
        /*!
        * Pause receiving of messages. This will prevent the transport from receiving
        * any more messages until it has been resumed. This is used to excert
        * backpressure on the sending peer.
        */
        virtual void pause() override;

        /*!
        * Resume receiving of messages. The transport will now begin receiving messsages
        * again and lift backpressure from the sending peer.
        */
        virtual void resume() override;

        /*!
        * @copydoc wamp_transport::attach()
        */
        virtual void attach(
            const std::shared_ptr<wamp_transport_handler>& handler) override;

        /*!
        * @copydoc wamp_transport::detach()
        */
        virtual void detach() override;

        /*!
        * @copydoc wamp_transport::has_handler()
        */
        virtual bool has_handler() const override;


    protected:
        virtual bool is_open() const = 0;


        virtual void async_connect(const std::string& m_uri, boost::promise<void>& connect_promise) = 0;
        virtual void close() = 0;

        virtual void write(void const * payload, size_t len) = 0;

        void receive_message(const std::string& msg);

        /*!
        * The promise that is fulfilled when the connect attempt is complete.
        */
        boost::promise<void> m_connect;

        /*!
        * The promise that is fulfilled when the disconnect attempt is complete.
        */
        boost::promise<void> m_disconnect;

    private:


        private:


            /*!
            * The handler to be called when pausing.
            */
            pause_handler m_pause_handler;

            /*!
            * The handler to be called when resuming.
            */
            resume_handler m_resume_handler;

            /*!
            * The transport handler to be notified of events/messages.
            */
            std::shared_ptr<wamp_transport_handler> m_handler;

            /*!
            * Used for unpacking serialized messages.
            */
            msgpack::unpacker m_message_unpacker;

            /*!
            * Whether or not debugging is enabled.
            */
            bool m_debug_enabled;
            
            /*!
            * Websocket endpoint URI
            */
            std::string m_uri;
    };
} // namespace autobahn

#include "wamp_websocket_transport.ipp"
#endif // AUTOBAHN_WEBSOCKET_TRANSPORT_HPP