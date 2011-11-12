#ifndef LIBSUB_DDS_RECEIVER_H
#define LIBSUB_DDS_RECEIVER_H

#include "LibSub/DDS/DDSException.h"
#include <ndds/ndds_cpp.h>
#include <boost/function.hpp>
#include <boost/shared_ptr.hpp>

namespace subjugator {
	template <class MessageT>
	class BaseReceiver {
		protected:
			typedef MessageT Message;
			typedef typename MessageTraits<MessageT>::DataReader MessageDataReader;
			typedef typename MessageTraits<MessageT>::Seq MessageSeq;

		public:
			BaseReceiver(Topic<Message> &topic)
			: topic(topic) {
				Participant &participant = topic.getParticipant();
				participant.registerType<Message>();

				DDSDataReader *reader = participant.getDDS().create_datareader(&topic.getDDS(), DDS_DATAREADER_QOS_USE_TOPIC_QOS, NULL, DDS_STATUS_MASK_NONE);
				if (!reader)
					throw DDSException("Failed to create DataReader");

				messagereader = MessageDataReader::narrow(reader);
				if (!messagereader)
					throw DDSException("Failed to narrow to MessageDataReader");
			}

			~BaseReceiver() {
				topic.getParticipant().getDDS().delete_datareader(messagereader);
			}

			Topic<Message> &getTopic() { return topic; }

		protected:
			Topic<Message> &topic;
			MessageDataReader *messagereader;
	};

	template <class MessageT>
	class PollingReceiver : public BaseReceiver<MessageT> {
		protected:
			using BaseReceiver<MessageT>::messagereader;
			typedef typename MessageTraits<MessageT>::TypeSupport TypeSupport;

		public:
			PollingReceiver(Topic<MessageT> &topic) : BaseReceiver<MessageT>(topic) { }

			boost::shared_ptr<MessageT> take() {
				typename BaseReceiver<MessageT>::MessageSeq messageseq;
				DDS_SampleInfoSeq infoseq;

				DDS_ReturnCode_t code = messagereader->take(messageseq, infoseq, 1, DDS_ANY_SAMPLE_STATE, DDS_ANY_VIEW_STATE, DDS_ANY_INSTANCE_STATE);
				if (code == DDS_RETCODE_NO_DATA)
					return boost::shared_ptr<MessageT>();
				else if (code != DDS_RETCODE_OK)
					throw DDSException("Failed to take from messagereader", code);

				assert(messageseq.length() == 1);

				boost::shared_ptr<MessageT> msg(TypeSupport::create_data(), &TypeSupport::delete_data);
				TypeSupport::copy_data(msg.get(), &messageseq[0]);
				messagereader->return_loan(messageseq, infoseq);

				return msg;
			}
	};

	template <class MessageT>
	class BlockingReceiver : public PollingReceiver<MessageT> {
		using PollingReceiver<MessageT>::messagereader;

		public:
			BlockingReceiver(Topic<MessageT> &topic) : PollingReceiver<MessageT>(topic) {
				DDSStatusCondition *cond = messagereader->get_statuscondition();
				cond->set_enabled_statuses(DDS_DATA_AVAILABLE_STATUS);
				waitset.attach_condition(cond);
			}

			void wait() {
				DDSConditionSeq seq;
				do {
					waitset.wait(seq, DDS_DURATION_INFINITE);
				} while (!seq.length());
			}

		private:
			DDSWaitSet waitset;

	};

	template <class MessageT>
	class Receiver : public BaseReceiver<MessageT>, DDSDataReaderListener {
		using BaseReceiver<MessageT>::messagereader;

		public:
			typedef boost::function<void (const MessageT &)> ReceiveCallback;
			typedef boost::function<void (int)> WriterCountCallback;

			Receiver(Topic<MessageT> &topic, const ReceiveCallback &receivecallback, const WriterCountCallback &writercountcallback)
			: BaseReceiver<MessageT>(topic), receivecallback(receivecallback), writercountcallback(writercountcallback) {
				DDS_ReturnCode_t code = messagereader->set_listener(this, DDS_DATA_AVAILABLE_STATUS | DDS_LIVELINESS_CHANGED_STATUS);
				if (code != DDS_RETCODE_OK)
					throw DDSException("Failed to set listener on the MessageDataReader", code);
			}

		private:
			ReceiveCallback receivecallback;
			WriterCountCallback writercountcallback;

			virtual void on_data_available(DDSDataReader *unused) {
				typename BaseReceiver<MessageT>::MessageSeq messageseq;
				DDS_SampleInfoSeq infoseq;
				DDS_ReturnCode_t code;

				code = messagereader->take(messageseq, infoseq, DDS_LENGTH_UNLIMITED, DDS_ANY_SAMPLE_STATE, DDS_ANY_VIEW_STATE, DDS_ANY_INSTANCE_STATE);
				if (code != DDS_RETCODE_OK)
					throw DDSException("Failed to take from DataReader", code);

				try {
					for (int i=0; i<messageseq.length(); ++i)
						receivecallback(messageseq[i]);

					messagereader->return_loan(messageseq, infoseq);
				} catch (...) {
					messagereader->return_loan(messageseq, infoseq);
					throw;
				}
			}

			virtual void on_liveliness_changed(DDSDataReader *unused, const DDS_LivelinessChangedStatus &status) {
				writercountcallback(status.alive_count);
			}
	};
}

#endif
