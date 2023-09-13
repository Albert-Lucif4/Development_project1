#include <ace/Log_Msg.h>

#include <dds/DdsDcpsInfrastructureC.h>
#include <dds/DdsDcpsSubscriptionC.h>

#include <dds/DCPS/Marked_Default_Qos.h>
#include <dds/DCPS/Service_Participant.h>
#include <dds/DCPS/WaitSet.h>
#include <string.h>
#include <dds/DCPS/StaticIncludes.h>
#ifdef ACE_AS_STATIC_LIBS
#  include <dds/DCPS/RTPS/RtpsDiscovery.h>
#  include <dds/DCPS/transport/rtps_udp/RtpsUdp.h>
#endif

#include "DataReaderListenerImpl.h"
#include "instructionTypeSupportImpl.h"


int
ACE_TMAIN(int argc, ACE_TCHAR *argv[])
{
	try {
		// Initialize DomainParticipantFactory
		DDS::DomainParticipantFactory_var dpf =
			TheParticipantFactoryWithArgs(argc, argv);

		// Create DomainParticipant
		DDS::DomainParticipant_var participant =
			dpf->create_participant(42,
				PARTICIPANT_QOS_DEFAULT,
				0,
				OpenDDS::DCPS::DEFAULT_STATUS_MASK);

		if (!participant) {
			ACE_ERROR_RETURN((LM_ERROR,
				ACE_TEXT("ERROR: %N:%l: main() -")
				ACE_TEXT(" create_participant failed!\n")),
				1);
		}

		// Register Type (Messenger::Message)
		instruction::instructTypeSupport_var ts =
			new instruction::instructTypeSupportImpl;

		if (ts->register_type(participant, "") != DDS::RETCODE_OK) {
			ACE_ERROR_RETURN((LM_ERROR,
				ACE_TEXT("ERROR: %N:%l: main() -")
				ACE_TEXT(" register_type failed!\n")),
				1);
		}
		
		// Create Topic (Movie Discussion List)
		
			CORBA::String_var type_name = ts->get_type_name();
			DDS::Topic_var topic =
				participant->create_topic("ÇýÖð½¢",
					type_name,
					TOPIC_QOS_DEFAULT,
					0,
					OpenDDS::DCPS::DEFAULT_STATUS_MASK);

			if (!topic) {
				ACE_ERROR_RETURN((LM_ERROR,
					ACE_TEXT("ERROR: %N:%l: main() -")
					ACE_TEXT(" create_topic failed!\n")),
					1);
			}

			// Create Subscriber
			DDS::Subscriber_var subscriber =
				participant->create_subscriber(SUBSCRIBER_QOS_DEFAULT,
					0,
					OpenDDS::DCPS::DEFAULT_STATUS_MASK);

			if (!subscriber) {
				ACE_ERROR_RETURN((LM_ERROR,
					ACE_TEXT("ERROR: %N:%l: main() -")
					ACE_TEXT(" create_subscriber failed!\n")),
					1);
			}

			// Create DataReader
			DDS::DataReaderListener_var listener(new DataReaderListenerImpl);

			DDS::DataReaderQos reader_qos;
			subscriber->get_default_datareader_qos(reader_qos);
			reader_qos.reliability.kind = DDS::RELIABLE_RELIABILITY_QOS;

			DDS::DataReader_var reader =
				subscriber->create_datareader(topic,
					reader_qos,
					listener,
					OpenDDS::DCPS::DEFAULT_STATUS_MASK);

			if (!reader) {
				ACE_ERROR_RETURN((LM_ERROR,
					ACE_TEXT("ERROR: %N:%l: main() -")
					ACE_TEXT(" create_datareader failed!\n")),
					1);
			}

			instruction::instructDataReader_var reader_i =
				instruction::instructDataReader::_narrow(reader);

			if (!reader_i) {
				ACE_ERROR_RETURN((LM_ERROR,
					ACE_TEXT("ERROR: %N:%l: main() -")
					ACE_TEXT(" _narrow failed!\n")),
					1);
			}

			// Block until Publisher completes
			DDS::StatusCondition_var condition = reader->get_statuscondition();
			condition->set_enabled_statuses(DDS::SUBSCRIPTION_MATCHED_STATUS);
			DDS::Duration_t timeout = { 5, 0 };
			DDS::ConditionSeq conditions;
			DDS::WaitSet_var ws = new DDS::WaitSet;
			

			while (true) {
				DDS::SubscriptionMatchedStatus matches;
				ws->attach_condition(condition);
				if (reader->get_subscription_matched_status(matches) != DDS::RETCODE_OK) {
					ACE_ERROR_RETURN((LM_ERROR,
						ACE_TEXT("ERROR: %N:%l: main() -")
						ACE_TEXT(" get_subscription_matched_status failed!\n")),
						1);
				}

				if (matches.current_count == 0 && matches.total_count > 0) {
					ws->detach_condition(condition);


				
				}

				if (ws->wait(conditions, timeout) != DDS::RETCODE_OK) {
					ACE_DEBUG((LM_DEBUG, ACE_TEXT("Waiting for a message...\n")));
				}
			}

			
	
		// Clean-up!
		participant->delete_contained_entities();
		dpf->delete_participant(participant);

		TheServiceParticipant->shutdown();

	}
	catch (const CORBA::Exception& e) {
		e._tao_print_exception("Exception caught in main():");
		return 1;
	}

	return 0;
}