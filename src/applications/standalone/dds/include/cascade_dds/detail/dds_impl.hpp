#include <bits/c++config.h>
#include <cascade_dds/dds.hpp>
#include <derecho/mutils-serialization/SerializationSupport.hpp>
#include <mutex>
#include <thread>
#include <deque>
#include <atomic>
namespace derecho {
namespace cascade {

#define DDS_CONFIG_JSON_FILE    "dds.json"

class DDSConfigJsonImpl: public DDSConfig {
    nlohmann::json config;
public:
    DDSConfigJsonImpl(const std::string& conf_file=DDS_CONFIG_JSON_FILE);
    std::string get_metadata_pathname() const override;
    std::vector<std::string> get_data_plane_pathnames() const override;
    std::string get_control_plane_suffix() const override;
    virtual ~DDSConfigJsonImpl();
};

template <typename T>
auto DDSMetadataClient::list_topics(const std::function<T(const std::unordered_map<std::string,Topic>&)>& func, bool refresh) {
    if (refresh) {
        refresh_topics();
    }
    std::shared_lock<std::shared_mutex> rlock(topics_shared_mutex);
    return func(topics);
}

/* The data plane message type */
class DDSMessage : mutils::ByteRepresentable {
public:
    /* topic */
    std::string topic;
    /* application data */
    Blob app_data;

    DEFAULT_SERIALIZATION_SUPPORT(DDSMessage,topic,app_data);

    /* constructors */
    DDSMessage();
    DDSMessage(const std::string& _topic,const Blob& _blob);
    DDSMessage(const DDSMessage& rhs);
    DDSMessage(DDSMessage&& rhs);
};

#define MAX_TOPIC_NAME_LENGTH    (32)
struct __attribute__((packed, aligned(4))) DDSMessageHeader {
    std::size_t topic_name_length;
    char        topic_name[MAX_TOPIC_NAME_LENGTH];
    uint8_t     message_bytes;
};
#define DDS_MESSAGE_HEADER_SIZE   offsetof(DDSMessageHeader,message_bytes)

template <typename MessageType>
class DDSPublisherImpl: public DDSPublisher<MessageType> {
    std::shared_ptr<ServiceClientAPI> capi;
    const std::string topic;
    const std::string cascade_key;
    static thread_local ObjectWithStringKey object_buffer;
    static thread_local bool object_buffer_initialized;
public:
    /** Constructor
     * @param _capi         The shared cascade client handle
     * @param _topic        The topic of the publisher
     */
    DDSPublisherImpl(
            const std::shared_ptr<ServiceClientAPI>& _capi,
            const std::string& _topic,
            const std::string& _object_pool) : 
        capi(_capi),topic(_topic),cascade_key(_object_pool+PATH_SEPARATOR+_topic) {
        if(topic.size() > MAX_TOPIC_NAME_LENGTH) {
            throw derecho::derecho_exception(
                    "the size of '" + topic + "' exceeds " + std::to_string(MAX_TOPIC_NAME_LENGTH));
        }
    }

    virtual const std::string& get_topic() const override {
        return topic;
    }

    virtual void send(const MessageType& message) override {
        if (!object_buffer_initialized) {
            object_buffer.key = cascade_key;
            object_buffer_initialized = true;
        }
        std::size_t requested_size = mutils::bytes_size(message) + DDS_MESSAGE_HEADER_SIZE;
        if (object_buffer.blob.capacity < (requested_size)) {
            uint8_t* bytes = new uint8_t[requested_size];
            // set up header
            DDSMessageHeader* msg_ptr = reinterpret_cast<DDSMessageHeader*>(bytes);
            std::memcpy(msg_ptr->topic_name,topic.c_str(),topic.size());
            msg_ptr->message_bytes = topic.size();
            // fill data
            mutils::to_bytes(message,msg_ptr->message_bytes);
            object_buffer.blob = std::move(Blob(bytes,requested_size));
            delete[] bytes;
        } else {
            DDSMessageHeader* msg_ptr = reinterpret_cast<DDSMessageHeader*>(object_buffer.blob.bytes);
            object_buffer.blob.size = mutils::to_bytes(message,msg_ptr->message_bytes);
        }
        // send message
        capi->put_and_forget(object_buffer);
    }

    /** Destructor */
    ~DDSPublisherImpl(){
        // nothing to release manually.
    }
};

template<typename MessageType>
thread_local ObjectWithStringKey DDSPublisherImpl<MessageType>::object_buffer;

template<typename MessageType>
thread_local bool DDSPublisherImpl<MessageType>::object_buffer_initialized=false;

class PerTopicRegistry;
class DDSSubscriberRegistry;

/**
 * @class SubscriberCore
 */
class SubscriberCore {
    friend PerTopicRegistry;
    friend DDSSubscriberRegistry;

private:
    const std::string topic;
    const uint32_t index;
    std::atomic<bool> online;
    std::unordered_map<std::string,cascade_notification_handler_t>  handlers;
    mutable std::mutex                                              handlers_mutex;

    std::deque<Blob>                message_queue;
    mutable std::condition_variable message_queue_cv;
    mutable std::mutex              message_queue_mutex;
    std::thread                     message_worker;

public:
    /**
     * The Constructor
     * @param _topic    topic
     * @param _index    the index in the per topic registry.
     */
    SubscriberCore(const std::string& _topic, const uint32_t _index);

    /**
     * add a handler
     * @param handler_name
     * @param handler
     */
    void add_handler(const std::string& handler_name, const cascade_notification_handler_t& handler);

    /**
     * list all handlers
     *
     * @return the name of the handlers
     */
    std::vector<std::string> list_handlers() const;

    /**
     * delete handler by name
     * @param handler name
     */
    void delete_handler(const std::string& handler_name);

    /**
     * Post a message to this subscriber.
     */
    void post(const Blob& blob);

    /**
     * shutdown the SubscriberCore thread
     * Please note that this will not remove it from the topic registry. Calling DDSClient::unsubscribe will do the
     * work.
     */
    void shutdown();

    /**
     * Destructor
     */
    virtual ~SubscriberCore();
};

/**
 * @class DDSSubscriberImpl
 */
template <typename MessageType>
class DDSSubscriberImpl: public DDSSubscriber<MessageType> {
    friend DDSSubscriberRegistry;
    std::shared_ptr<SubscriberCore> core;
public:
    /** Constructor
     * @param _capi         The shared cascade client handle
     * @param _topic        The topic of the publisher
     */
    DDSSubscriberImpl(const std::shared_ptr<SubscriberCore>& _core):
        core(_core) {}

    /**
     * add handler
     * @param handler_name
     * @param handler
     */
    void add_handler(const std::string& handler_name,const message_handler_t<MessageType>& handler) {
        core->add_handler(handler_name,
            [handler](const Blob& blob)->void {
                mutils::deserialize_and_run(nullptr,blob.bytes,handler);
        });
    }

    /**
     * list handlers
     * @return the list of handlers.
     */
    std::vector<std::string> list_handlers() const {
        return core->list_handlers();
    }

    /**
     * delete handler by name
     * @param handler_name
     */
    void delete_handler(const std::string& handler_name) {
        core->delete_handler(handler_name);
    }
};


class PerTopicRegistry {
    friend DDSSubscriberRegistry;
    std::string topic;
    std::string cascade_key;
    std::map<uint32_t,std::shared_ptr<SubscriberCore>> registry;
    mutable uint32_t counter;

    /* constructor */
    PerTopicRegistry(const std::string& _topic,
            const std::string _cascade_key):
        topic(_topic),
        cascade_key(_cascade_key),
        registry{},
        counter(0) {}

    /** 
     * Insert a new subscriber core, assuming the big lock (DDSSubscriberRegistry::registry_mutex)
     * 
     * @tparam  MessageType
     * @param   handlers        an optional set of named handlers to go with the initial SubscriberCore. 
     *
     * @return a shared pointer to the new created SubscriberCore
     */
    template <typename MessageType>
    std::shared_ptr<SubscriberCore> create_subscriber_core(const std::unordered_map<std::string,message_handler_t<MessageType>>& handlers={}) {
        //2 - create a subscriber core
        registry.emplace(counter, std::make_shared<SubscriberCore>(topic));
        for (const auto& handler: handlers) {
            registry[counter]->add_handler(handler.first,
                [handler](const Blob& blob){
                    mutils::deserialize_and_run(nullptr,blob.bytes,handler.second);
                }
            );
        }
        counter ++;
        return registry[counter-1];
    }
};

/**
 * The core of DDSClient, it manages the subscribers.
 */
class DDSSubscriberRegistry {
    const std::string control_plane_suffix;
    std::unordered_map<std::string,PerTopicRegistry> registry;
    mutable std::mutex registry_mutex;

    /* helpers */
    void _topic_control(std::shared_ptr<ServiceClientAPI>& capi, const Topic& topic_info, DDSCommand::CommandType command_type);

public:
    /**
     * Create a subscriber
     * @tparam  MessageType         the message type
     * @param   capi                the shared cascade client
     * @param   metadata_service    the dds metadata service
     * @param   topic               the topic
     * @param   handlers            an unordered map for the named handlers.
     *
     * @return a subscriber
     */
    template <typename MessageType>
    std::unique_ptr<DDSSubscriber<MessageType>> subscribe(
            std::shared_ptr<ServiceClientAPI>& capi,
            DDSMetadataClient& metadata_service,
            const std::string& topic,
            const std::unordered_map<std::string,message_handler_t<MessageType>>& handlers) {
        // apply the big lock
        std::lock_guard<std::mutex> lck(registry_mutex);
        // check if topic is registered.
        if (registry.find(topic)==registry.cend()) {
            // create a topic entry.
            auto topic_info = metadata_service.get_topic(topic);
            registry.emplace(topic,PerTopicRegistry(topic,topic_info.pathname));
            // register universal per-topic handler, which dispatches messages to subscriber cores.
            capi->register_notification_handler(
                    [this,topic](const Blob& blob)->void{
                        for(auto& subscriber_core:registry[topic].registry) {
                            if (subscriber_core.second->online) {
                                subscriber_core.second->post(blob);
                            }
                        }
                    },topic_info.pathname);
            // subscribe to a cascade server.
            _topic_control(capi,topic_info,DDSCommand::SUBSCRIBE);
        }
        // register the handlers
        auto subscriber_core = registry[topic].template create_subscriber_core<MessageType>(handlers);
        // create a Subscriber object wrapping the Subscriber Core
        return std::make_unique<DDSSubscriberImpl<MessageType>>(subscriber_core);
    }

    /**
     * Unsubscribe
     * @tparam MessageType      the message type
     * @param  subscriber       the subscriber to unsubscribe
     */
    template <typename MessageType>
    void unsubscribe(
            std::shared_ptr<ServiceClientAPI>& capi,
            DDSMetadataClient& metadata_service,
            DDSSubscriber<MessageType>& subscriber) {
        // apply the big lock
        std::lock_guard<std::mutex> lck(registry_mutex);
        DDSSubscriberImpl<MessageType>* impl = dynamic_cast<DDSSubscriberImpl<MessageType>*>(&subscriber);
        // test the existence of corresponding subscriber core.
        if (registry.find(impl->core.topic) == registry.cend()) {
            dbg_default_warn("unsubscribe abort because subscriber's topic '{}' does not exist in registry.", impl->core.topic);
            return;
        }
        if (registry[impl->core.topic].registry.find(impl->core.index) == registry[impl->core.topic].registry.cend()) {
            dbg_default_warn("unsubscribe abort because subscriber's index '{}' does not exist in the per topic registry.", impl->core.index);
            return;
        }
        // remove the subscriber core
        registry[impl->core.topic].registry.erase(impl->core.index);
        if (registry[impl->core.topic].registry.empty()) {
            auto topic_info = metadata_service.get_topic(impl->core.topic);
            // unsubscribe from a cascade server
            _topic_control(capi,topic_info,DDSCommand::UNSUBSCRIBE);
            // remove the topic entry
            registry.erase(impl->core.topic);
        }
    }

    /**
     * Destructor
     */
    virtual ~DDSSubscriberRegistry() {}
};

}
}
