#include "workerrequest.h"

namespace sonic_socket
{

const WorkerResolveRequest &WorkerRequest::to_resolve() const
{
    assert(is_resolve());
    return *static_cast<const WorkerResolveRequest*>(this);
}

WorkerDecodeRequest &WorkerRequest::to_decode()
{
    assert(is_decode());
    return *static_cast<WorkerDecodeRequest*>(this);
}

const WorkerEncodeRequest &WorkerRequest::to_encode() const
{
    assert(is_encode());
    return *static_cast<const WorkerEncodeRequest*>(this);
}

}
