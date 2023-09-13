#include <ace/Log_Msg.h>
#include <ace/OS_NS_stdlib.h>

#include "DataReaderListenerImpl.h"
#include "instructionTypeSupportC.h"
#include "instructionTypeSupportImpl.h"

#include <iostream>

void
DataReaderListenerImpl::on_requested_deadline_missed(
	DDS::DataReader_ptr /*reader*/,
	const DDS::RequestedDeadlineMissedStatus& /*status*/)
{
}

void
DataReaderListenerImpl::on_requested_incompatible_qos(
	DDS::DataReader_ptr /*reader*/,
	const DDS::RequestedIncompatibleQosStatus& /*status*/)
{
}

void
DataReaderListenerImpl::on_sample_rejected(
	DDS::DataReader_ptr /*reader*/,
	const DDS::SampleRejectedStatus& /*status*/)
{
}

void
DataReaderListenerImpl::on_liveliness_changed(
	DDS::DataReader_ptr /*reader*/,
	const DDS::LivelinessChangedStatus& /*status*/)
{
}

void
DataReaderListenerImpl::on_data_available(DDS::DataReader_ptr reader)
{
	instruction::instructDataReader_var reader_i =
		instruction::instructDataReader::_narrow(reader);

	if (!reader_i) {
		ACE_ERROR((LM_ERROR,
			ACE_TEXT("ERROR: %N:%l: on_data_available() -")
			ACE_TEXT(" _narrow failed!\n")));
		ACE_OS::exit(1);
	}

	instruction::instruct message;
	DDS::SampleInfo info;

	DDS::ReturnCode_t error = reader_i->take_next_sample(message, info);

	if (error == DDS::RETCODE_OK) {
		std::cout << "SampleInfo.sample_rank = " << info.sample_rank << std::endl;
		std::cout << "SampleInfo.instance_state = " << info.instance_state << std::endl;

		if (info.valid_data) {
			std::cout << "Message: subject    = " << message.vehicle.in() << std::endl
				<< "         subject_id = " << message.id << std::endl
				<< "         from       = " << message.action.in() << std::endl
				<< "         count      = " << message.distance << std::endl
				<< "         text       = " << message.dirction.in() << std::endl;

		}

	}
	else {
		ACE_ERROR((LM_ERROR,
			ACE_TEXT("ERROR: %N:%l: on_data_available() -")
			ACE_TEXT(" take_next_sample failed!\n")));
	}
}

void
DataReaderListenerImpl::on_subscription_matched(
	DDS::DataReader_ptr /*reader*/,
	const DDS::SubscriptionMatchedStatus& /*status*/)
{
}

void
DataReaderListenerImpl::on_sample_lost(
	DDS::DataReader_ptr /*reader*/,
	const DDS::SampleLostStatus& /*status*/)
{
}