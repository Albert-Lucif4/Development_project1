#include <ace/Log_Msg.h>

#include <dds/DdsDcpsInfrastructureC.h>
#include <dds/DdsDcpsPublicationC.h>

#include <dds/DCPS/Marked_Default_Qos.h>
#include <dds/DCPS/Service_Participant.h>
#include <dds/DCPS/WaitSet.h>
#include <cstring>
#include <dds/DCPS/StaticIncludes.h>
#ifdef ACE_AS_STATIC_LIBS
#  include <dds/DCPS/RTPS/RtpsDiscovery.h>
#  include <dds/DCPS/transport/rtps_udp/RtpsUdp.h>
#endif

#include "instructionTypeSupportImpl.h"
instruction::instruct message[2] = { {"驱逐舰","前进",99,"前进",100},{"护卫舰","原地待命",100,"原地",0} };
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

		// Register TypeSupport (Messenger::Message)
		instruction::instructTypeSupport_var ts =
			new instruction::instructTypeSupportImpl;

		if (ts->register_type(participant, "") != DDS::RETCODE_OK) {
			ACE_ERROR_RETURN((LM_ERROR,
				ACE_TEXT("ERROR: %N:%l: main() -")
				ACE_TEXT(" register_type failed!\n")),
				1);
		}
		char *top[2] = { "驱逐舰", "护卫舰" };
		// Create Topic (Movie Discussion List)
		for (int i = 0; i < 2; i++)
		{
			CORBA::String_var type_name = ts->get_type_name();
			DDS::Topic_var topic =
				participant->create_topic(top[i],
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

			// Create Publisher
			DDS::Publisher_var publisher =
				participant->create_publisher(PUBLISHER_QOS_DEFAULT,
					0,
					OpenDDS::DCPS::DEFAULT_STATUS_MASK);

			if (!publisher) {
				ACE_ERROR_RETURN((LM_ERROR,
					ACE_TEXT("ERROR: %N:%l: main() -")
					ACE_TEXT(" create_publisher failed!\n")),
					1);
			}

			// Create DataWriter
			DDS::DataWriter_var writer =
				publisher->create_datawriter(topic,
					DATAWRITER_QOS_DEFAULT,
					0,
					OpenDDS::DCPS::DEFAULT_STATUS_MASK);

			if (!writer) {
				ACE_ERROR_RETURN((LM_ERROR,
					ACE_TEXT("ERROR: %N:%l: main() -")
					ACE_TEXT(" create_datawriter failed!\n")),
					1);
			}

			instruction::instructDataWriter_var message_writer =
				instruction::instructDataWriter::_narrow(writer);

			if (!message_writer) {
				ACE_ERROR_RETURN((LM_ERROR,
					ACE_TEXT("ERROR: %N:%l: main() -")
					ACE_TEXT(" _narrow failed!\n")),
					1);
			}


			// Block until Subscriber is available
			DDS::StatusCondition_var condition = writer->get_statuscondition();
			condition->set_enabled_statuses(DDS::PUBLICATION_MATCHED_STATUS);

			DDS::WaitSet_var ws = new DDS::WaitSet;
			ws->attach_condition(condition);

			ACE_DEBUG((LM_DEBUG,
				ACE_TEXT("Block until subscriber is available\n")));

			while (true) {
				DDS::PublicationMatchedStatus matches;
				if (writer->get_publication_matched_status(matches) != ::DDS::RETCODE_OK) {
					ACE_ERROR_RETURN((LM_ERROR,
						ACE_TEXT("ERROR: %N:%l: main() -")
						ACE_TEXT(" get_publication_matched_status failed!\n")),
						1);
				}

				if (matches.current_count >= 1) {
					break;
				}

				DDS::ConditionSeq conditions;
				DDS::Duration_t timeout = { 60, 0 };
				if (ws->wait(conditions, timeout) != DDS::RETCODE_OK) {
					ACE_ERROR_RETURN((LM_ERROR,
						ACE_TEXT("ERROR: %N:%l: main() -")
						ACE_TEXT(" wait failed!\n")),
						1);
				}
			}

			ACE_DEBUG((LM_DEBUG,
				ACE_TEXT("Subscriber is available\n")));

			ws->detach_condition(condition);
			
			// Write samples
			/*instruction::instruct message1, message2;
			message1.id = a[i].id;
			message1.vehicle = "驱逐舰";
			message1.action = "前进";
			message1.dirction = "西北方向";
			message1.distance = 100;*/
			/*message2.id = 100;
			message2.vehicle = "护卫舰";
			message2.action = "原地待命";
			message2.direction = "原地";
			message2.distance = 0;*/
			/*for (int i = 0; i < 10; ++i) {
				DDS::ReturnCode_t error = message_writer->write(message, DDS::HANDLE_NIL);
				++message.count;
				++message.subject_id;

				if (error != DDS::RETCODE_OK) {
					ACE_ERROR((LM_ERROR,
						ACE_TEXT("ERROR: %N:%l: main() -")
						ACE_TEXT(" write returned %d!\n"), error));
				}
			}*/
			DDS::ReturnCode_t error = message_writer->write(message[i], DDS::HANDLE_NIL);
			//DDS::ReturnCode_t error = message_writer2->write(message2, DDS::HANDLE_NIL);
			// Wait for samples to be acknowledged
			DDS::Duration_t timeout = { 30, 0 };
			if (message_writer->wait_for_acknowledgments(timeout) != DDS::RETCODE_OK) {
				ACE_ERROR_RETURN((LM_ERROR,
					ACE_TEXT("ERROR: %N:%l: main() -")
					ACE_TEXT(" wait_for_acknowledgments failed!\n")),
					1);
			}

			// Clean-up!
		}
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