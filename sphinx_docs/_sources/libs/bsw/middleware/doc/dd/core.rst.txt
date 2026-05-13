Core
====

The core software unit of the middleware provides the key algorithms needed for the communication between applications.

Middleware Message
------------------

Middleware is a service-oriented message passing system, and as such the ``Message`` class is the fundamental unit of communication.
This class consists of a header and a payload.
The header has the following information:

* Service ID - identifies the service to which the message belongs.
* Member ID - identifies the specific member function of the service that is being invoked.
* Request ID - identifies the specific request being made.
* Instance ID - identifies the instance of the service (multiple instances may exist).
* Source cluster ID - identifies the cluster from which the message originated.
* Destination Cluster ID - identifies the cluster to which the message is being sent.
* Address ID - identifies the recipient of the message within the destination cluster.

The middleware services' nodes will be scattered across different clusters in the ECU.
This means that messages can travel between clusters, and as such the ``Message`` needs to have information about its origin and destination.
Additionally, there may exist several possible recipients of a message, and as such, each recipient needs to have a unique identifier after system initialization.
To this end, the header contains the source cluster ID, target cluster ID, and address ID fields.

Finally, the payload contains the actual data being transmitted. This payload can be up to MAX_PAYLOAD_SIZE (currently 20 bytes long) and is stored directly within the ``Message`` object.
If the payload exceeds this size, the middleware employs its own memory management system and stores an external handle within the ``Message`` object.
This handle contains information about the location and size of the payload in the middleware's memory region, as well as a flag indicating whether the payload is shared among multiple messages.
In case of internal errors, the payload can store an error code instead, which is delivered to any recipient waiting for a response.
