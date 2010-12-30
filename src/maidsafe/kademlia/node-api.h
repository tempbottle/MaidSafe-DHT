/* Copyright (c) 2009 maidsafe.net limited
All rights reserved.

Redistribution and use in source and binary forms, with or without modification,
are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice,
    this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice,
    this list of conditions and the following disclaimer in the documentation
    and/or other materials provided with the distribution.
    * Neither the name of the maidsafe.net limited nor the names of its
    contributors may be used to endorse or promote products derived from this
    software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR
TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

/*******************************************************************************
 * NOTE: This API is unlikely to have any breaking changes applied.  However,  *
 *       it should not be regarded as a final API until this notice is removed.*
 ******************************************************************************/

#ifndef MAIDSAFE_KADEMLIA_NODE_API_H_
#define MAIDSAFE_KADEMLIA_NODE_API_H_

#include <maidsafe/maidsafe-dht_config.h>
#include "maidsafe/kademlia/config.h"
#include <maidsafe/kademlia/nodeid.h>
#include <string>
#include <vector>

#if MAIDSAFE_DHT_VERSION < 25
#error This API is not compatible with the installed library.
#error Please update the maidsafe-dht library.
#endif


namespace base {
class AlternativeStore;
class SignatureValidator;
}  // namespace base


namespace transport {
class Transport;
class UdtTransport;
}  // namespace transport

namespace kademlia {

class Contact;
class ContactInfo;
class NodeImpl;
class Rpcs;
struct NodeConstructionParameters;

namespace protobuf {
class SignedValue;
class Signature;
}  // namespace protobuf

/**
* @class Node
* This class represents a kademlia node providing the API to join the network,
* find nodes and values, store and delete values, ping nodes, as well as the
* methods to access the local storage of the node and its routing table  .
*/

class Node {
 public:
  /**
  * Constructor that takes by default the kaemlia values for k, alpha, beta, and
  * refresh time.
  * @param transport a pointer to the transport object in charge of transmitting
  * data from the node to a specific node
  * @param type the type of node VAULT or CLIENT (client is read only node)
  * @param private_key private key for the node, if no digitally signed values
  * are used, pass an empty string
  * @param public_key public key for the node, if no digitally signed values
  * are used, pass an empty string
  * @param port_forwarded indicate if the port where the Transport object is
  * listening has been manually forwarded in the router
  * @param use_upnp indicate if UPnP is going to be used as the first option
  * for NAT traversal
  * @param k Maximum number of elements in the node's kbuckets
  */
  Node(boost::shared_ptr<transport::Transport> transport,
        const NodeConstructionParameters &node_parameters);

  ~Node();

  /**
  * Join the network using a specific id. This is a non-blocking operation.
  * @param node_id Id that is going to be used by the node
  * @param kad_config_file path to the config file where bootstrapping nodes are
  * stored.
  * @param callback callback function where result of the operation is notified
  */
  void Join(const NodeId &node_id, const std::string &kad_config_file,
            VoidFunctorOneString callback);
  /**
  * Join the network using a random id. This is a non-blocking operation.
  * @param kad_config_file path to the config file where bootstrapping nodes are
  * stored.
  * @param callback callback function where result of the operation is notified
  */
  void Join(const std::string &kad_config_file, VoidFunctorOneString callback);
  /**
  * Join the first node of the network using a specific id.
  * This is a non-blocking operation.
  * @param node_id Id that is going to be used by the node
  * @param kad_config_file path to the config file where bootstrapping nodes are
  * stored.
  * @param ip external ip of the node
  * @param port external port of the node
  * @param callback callback function where result of the operation is notified
  */
  void JoinFirstNode(const NodeId &node_id, const std::string &kad_config_file,
                     const IP &ip, const Port &port,
                     VoidFunctorOneString callback);
  /**
  * Join the first node of the network using a random id.
  * This is a non-blocking operation.
  * @param kad_config_file path to the config file where bootstrapping nodes are
  * stored.
  * @param ip external ip of the node
  * @param port external port of the node
  * @param callback callback function where result of the operation is notified
  */
  void JoinFirstNode(const std::string &kad_config_file, const IP &ip,
                     const Port &port, VoidFunctorOneString callback);
  /**
  * Leave the kademlia network.  All values stored in the node are erased and
  * nodes from the routing table are saved as bootstrapping nodes in the
  * config file
  */
  void Leave();
  /**
  * Store a value of the form (data; signed data) in the network.  Used if the
  * network is formed by nodes that have private and public key.
  * @param key a kademlia::NodeId object that is the key to store the value
  * @param signed_value signed value to be stored
  * @param signed_request request to store the value,
           it is validated before the value is stored
  * @param ttl time to live of the value in seconds, if ttl = -1, then it has
  * infinite time to live
  * @param callback callback function where result of the operation is notified
  */
  void StoreValue(const NodeId &key, const protobuf::SignedValue &signed_value,
                  const protobuf::Signature &signed_request,
                  const boost::int32_t &ttl, VoidFunctorOneString callback);
  /**
  * Store a value (a simple string) in the network.  Used if the
  * network is formed by nodes that do not have private and public key.
  * @param key a kademlia::NodeId object that is the key to store the value
  * @param value value to be stored
  * @param ttl time to live of the value in seconds, if ttl = -1, then it has
  * infinite time to live
  * @param callback callback function where result of the operation is notified
  */
  void StoreValue(const NodeId &key, const std::string &value,
                  const boost::int32_t &ttl, VoidFunctorOneString callback);
  /**
  * Delete a Value of the network, only in networks with nodes that have public
  * and private keys a value, that is of the form data; signed data, can be
  * deleted.  Only the one who signed the value can delete it.
  * @param key kademlia::NodeId object that is the key under which the value is stored
  * @param signed_value signed value to be deleted
  * @param signed_request request to delete the value, it is validated before the
  * value is deleted
  * @param callback callback function where result of the operation is notified
  */
  void DeleteValue(const NodeId &key, const protobuf::SignedValue &signed_value,
                   const protobuf::Signature &signed_request,
                   VoidFunctorOneString callback);
  /**
  * Update a Value of the network, only in networks with nodes that have public
  * and private keys a value, that is of the form <data, signed data>, can be
  * updated.  Only the one who signed the value can update it.
  * @param key kademlia::NodeId object that is the key under which the value is stored
  * @param old_value signed value to be updated
  * @param new_value signed value to be updated
  * @param signed_request request to update the value, it is validated before the
  * value is updated
  * @param callback callback function where result of the operation is notified
  */
  void UpdateValue(const NodeId &key,
                   const protobuf::SignedValue &old_value,
                   const protobuf::SignedValue &new_value,
                   const protobuf::Signature &signed_request,
                   boost::uint32_t ttl,
                   VoidFunctorOneString callback);
  /**
  * Find a value in the network.  If several values are stored under the same
  * key, a list with all the values is returned.
  * If any Node during the iterative lookup has the value in its
  * AlternativeStore, rather than returning this value, it returns its own
  * contact details.  If check_alternative_store is true, this node checks its own
  * AlternativeStore also.
  * @param key kademlia::NodeId object that is the key under which the value is stored
  * @param check_alternative_store indicate if the node's alternative store must be
  * checked
  * @param callback callback function where result of the operation is notified
  */
  void FindValue(const NodeId &key, const bool &check_alternative_store,
                 VoidFunctorOneString callback);
  /**
  * Find the contact details of a node in the network with its id.
  * @param node_id id of the node. It is a kademlia::NodeId object
  * @param callback callback function where result of the operation is notified
  * @param local false if the we want to find the node in the network and true
  * if we try to find it in the node's routing table
  */
  void GetNodeContactDetails(const NodeId &node_id,
                             VoidFunctorOneString callback, const bool &local);
  /**
  * Find the k closest nodes to an id in the network.
  * @param node_id id to which the nodes closest to it are returned
  * @param callback callback function where result of the operation is notified
  */
  void FindKClosestNodes(const NodeId &node_id, VoidFunctorOneString callback);
  /**
  * Find the k closest nodes to a key in the node's routing table.
  * @param key id to which the nodes closest to it are returned
  * @param exclude_contacts vector of nodes that must be excluded from the
  * result
  * @param close_nodes reference to a vector of Contact where the nodes found
  * are returned
  */
  void GetNodesFromRoutingTable(const NodeId &key,
                                 const std::vector<Contact> &exclude_contacts,
                                 std::vector<Contact> *close_nodes);
  /**
  * Ping the node with id node_id.  First the node is found in the network, and
  * then the node is pinged
  * @param node_id id of the node
  * @param callback callback function where result of the operation is notified
  */
  void Ping(const NodeId &node_id, VoidFunctorOneString callback);
  /**
  * Ping a node.
  * @param remote contact info of the node to be pinged
  * @param callback callback function where result of the operation is notified
  */
  void Ping(const Contact &remote, VoidFunctorOneString callback);
  /**
  * Add a node to the routing table and/or to the database routing table.
  * @param new_contact contact info of the node to be added
  * @param rtt Round trip time to the node
  * @param only_db if true, it is only added to the database routing table.
  */
  int AddContact(Contact new_contact, const float &rtt, const bool &only_db);
  /**
  * Remove a node from the routing table.
  * @param node_id id of the node
  */
  void RemoveContact(const NodeId &node_id);
  /**
  * Get a node from the routing table.
  * @param id id of the node
  * @param contact reference to a Contact object where the contact info of the
  * node is returned
  * @return True if node is found, false otherwise
  */
  bool GetContact(const NodeId &id, Contact *contact);
  /**
  * Find a value in the local data store of the node.
  * @param key key used to find the value
  * @param values vector of references where the values stored under key,
    if found, are retured
  * @return True if value is found, false otherwise
  */
  bool FindValueLocal(const NodeId &key, std::vector<std::string> *values);
  /**
  * Store a value in the local data store of the node.
  * @param key key under which the value is stored
  * @param value value to be stored
  * @param ttl Time to live of the value in seconds, if ttl = -1, then it has
  * infinite time to live
  * @return True if value is found, false otherwise
  */
  bool StoreValueLocal(const NodeId &key, const std::string &value,
                       const boost::int32_t &ttl);
  /**
  * Refhresh a value in the local data store of the node.  If the value was
  * already stored, the time to live is not changed, only the refresh time
  * is updated.
  * @param key key under which the value is stored
  * @param value value to be stored
  * @param ttl Time to live of the value in seconds, if ttl = -1, then it has
  * infinite time to live
  * @return True if value is found, false otherwise
  */
  bool RefreshValueLocal(const NodeId &key, const std::string &value,
                         const boost::int32_t &ttl);
  /**
  * Get n random nodes from the routing table.
  * @param count number of nodes to be returned
  * @param exclude_contacts nodes vector of nodes that cant be included in the
  * result
  * @param contacts reference to a vector of Contact where the nodes selected
  * are returned
  */
  void GetRandomContacts(const size_t &count,
                         const std::vector<Contact> &exclude_contacts,
                         std::vector<Contact> *contacts);
  /**
  * Notifier that is passed to the transport object for the case where the
  * node's randezvous server goes down.
  * @param dead_server notification of status of the rendezvous server: True
  * server is up, False server is down
  */
  void HandleDeadRendezvousServer(const bool &dead_server);
  /**
  * Check if the local endpoint corresponding to the local ip and port of a node
  * can be contacted if it is not already marked in the database routing table.
  * If the status is in the database routing table, it returns that status.
  * @param id id of the node we are checking
  * @param ip local ip
  * @param port local port
  * @param ext_ip external ip of the node
  * @return If the node can be contacted through its local endpoint (LOCAL) or
  * not (REMOTE)
  */
  ConnectionType CheckContactLocalAddress(const NodeId &id,
                                          const IP &ip,
                                          const Port &port,
                                          const IP &ext_ip);
  /**
  * Updates the database routing table in the entry for the node id passed to be
  * to be contacted only via the remote endpoint.
  * @param node_id id of the node
  * @param ip ip of the node
  */
  void UpdatePDRTContactToRemote(const NodeId &node_id,
                                 const IP &ip);
  Contact contact_info() const;
  NodeId node_id() const;
  IP ip() const;
  Port port() const;
  IP local_ip() const;
  Port local_port() const;
  IP rendezvous_ip() const;
  Port rendezvous_port() const;
  bool is_joined() const;
  /**
  * Returns a shared pointer to Rpcs
  * @return Shared pointer to Rpcs
  */
  boost::shared_ptr<Rpcs> rpcs();
  /**
  * Get the time of the last time a key/value pair stored in the node was
  * refreshed
  * @param key key under which the value is stored
  * @param value value stored
  * @return time in seconds from epoch time when the key/value pair was
  * refreshed.  It key value is not found, then 0 is returned.
  */
  boost::uint32_t KeyLastRefreshTime(const NodeId &key,
                                     const std::string &value);
  /**
  * Get the time when a key/value pair stored in the node is going to expire
  * @param key key under which the value is stored
  * @param value value stored
  * @return time in seconds from epoch time when the key/value pair is going to
  * expire.  It key value is not found, then 0 is returned.  If -1 is returned,
  * then the value doesn't expire
  */
  boost::uint32_t KeyExpireTime(const NodeId &key, const std::string &value);
  /**
  * Checks if the node has public and private RSA keys.
  * @return True if the node has the keys, otherwise False.
  */
  bool using_signatures();
  /**
  * Get the time to live of a key/value pair stored in the node
  * @param key key under which the value is stored
  * @param value value stored
  * @return time to live in seconds of the key/value. It key value is not found,
  * then 0 is returned.  If -1 is returned, then the value doesn't expire
  */
  boost::int32_t KeyValueTTL(const NodeId &key, const std::string &value) const;
  /**
  * If this is set to a non-NULL value, then the AlternativeStore will be used
  * before Kad's native DataStore.
  * @param alternative_store reference to a base::AlternativeStore object
  */
  void set_alternative_store(base::AlternativeStore *alternative_store);
  base::AlternativeStore *alternative_store();
  void set_signature_validator(base::SignatureValidator *validator);

 private:
  boost::shared_ptr<NodeImpl> pimpl_;
};

}  // namespace kademlia

#endif  // MAIDSAFE_KADEMLIA_NODE_API_H_
