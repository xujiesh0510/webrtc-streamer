/* ---------------------------------------------------------------------------
** This software is in the public domain, furnished "as is", without technical
** support, and with no warranty, express or implied, as to its usefulness for
** any purpose.
**
** rtspaudiocapturer.h
**
** -------------------------------------------------------------------------*/

#pragma once

#include <iostream>
#include <thread>
#include <mutex>
#include <queue>

#include "environment.h"
#include "mkvclient.h"

#include "pc/local_audio_source.h"
#include "api/audio_codecs/builtin_audio_decoder_factory.h"

class FileAudioSource : public webrtc::Notifier<webrtc::AudioSourceInterface>, public MKVClient::Callback {
	public:
		static rtc::scoped_refptr<FileAudioSource> Create(rtc::scoped_refptr<webrtc::AudioDecoderFactory> audioDecoderFactory, const std::string & uri, const std::map<std::string,std::string> & opts) {
			rtc::scoped_refptr<FileAudioSource> source(new rtc::RefCountedObject<FileAudioSource>(audioDecoderFactory, uri, opts));
			return source;
		}

		SourceState state() const override { return kLive; }
		bool remote() const override { return true; }
		
		virtual void AddSink(webrtc::AudioTrackSinkInterface* sink) override {
			RTC_LOG(INFO) << "FileAudioSource::AddSink ";
			std::lock_guard<std::mutex> lock(m_sink_lock);
			m_sinks.push_back(sink);
		}
		virtual void RemoveSink(webrtc::AudioTrackSinkInterface* sink) override {
			RTC_LOG(INFO) << "FileAudioSource::RemoveSink ";
			std::lock_guard<std::mutex> lock(m_sink_lock);
			m_sinks.remove(sink);
		}

		void CaptureThread() { m_env.mainloop(); } 		

		// overide RTSPConnection::Callback
		virtual bool onNewSession(const char* id, const char* media, const char* codec, const char* sdp) override;		
		virtual bool onData(const char* id, unsigned char* buffer, ssize_t size, struct timeval presentationTime) override;
		
	protected:
		FileAudioSource(rtc::scoped_refptr<webrtc::AudioDecoderFactory> audioDecoderFactory, const std::string & uri, const std::map<std::string,std::string> & opts); 
		virtual ~FileAudioSource();

	private:
		std::thread                                     m_capturethread;
		Environment                                     m_env;
		MKVClient                                       m_connection; 
		rtc::scoped_refptr<webrtc::AudioDecoderFactory> m_factory;
		std::unique_ptr<webrtc::AudioDecoder>           m_decoder;
		int                                             m_freq;
		int                                             m_channel;
		std::queue<uint16_t>                            m_buffer;
		std::list<webrtc::AudioTrackSinkInterface*>     m_sinks;
		std::mutex                                      m_sink_lock;
};


