#include "data.h"

XBT_LOG_NEW_DEFAULT_CATEGORY(Data_Transfer, "Messages specific for this s4u example");

void Data::sender(int data_count, size_t payload_size)
{

  sg4::Mailbox* mbox = sg4::Mailbox::by_name("receiver");

  for (int i = 0; i < data_count; i++) {
    std::string data_content = "data_" + std::to_string(i);
    auto* payload = new std::string(data_content);
    sg4::CommPtr comm = mbox->put_async(payload, payload_size);
    XBT_INFO("Send '%s' to '%s'", data_content.c_str(), mbox->get_cname());
    comm->wait();
  }

  /* Send message to let the receiver know that it should stop */
  XBT_INFO("Send 'finalize' to 'receiver'");
  mbox->put(new std::string("finalize"), 0);
}

/* Receiver actor expects 1 argument: its ID */
void Data::receiver()
{

  sg4::Mailbox* mbox = sg4::Mailbox::by_name("receiver");

  for (bool cont = true; cont;) {
    std::string* received;
    sg4::CommPtr comm = mbox->get_async<std::string>(&received);
    comm->wait();
    XBT_INFO("I got a '%s'.", received->c_str());
    if (*received == "finalize") cont = false; 
    delete received;
  }
}
