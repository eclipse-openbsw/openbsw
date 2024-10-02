// Copyright 2024 Accenture.

#ifndef GUARD_205C0900_CB38_4D9D_BD89_0825E803775C
#define GUARD_205C0900_CB38_4D9D_BD89_0825E803775C

#include "util/format/StringWriter.h"
#include "util/stream/ISharedOutputStream.h"

namespace util
{
namespace format
{
/**
 * A simple StringWriter that allocates a temporary stream from a shared stream on construction
 * and releases this stream on destruction.
 * \note while output on shared streams is exclusive the returned output stream should be held
 * only for a short while. Thus a shared string writer object typically will be instantiated
 * from within a non-blocking function call.
 */
class SharedStringWriter : public StringWriter
{
public:
    /**
     * constructor. The output stream is allocated (exclusively) by a call to startOutput
     * and kept during the lifetime of this object.
     * \param stream the shared output stream to allocate the output stream
     */
    explicit SharedStringWriter(::util::stream::ISharedOutputStream& strm);
    /**
     * destructor. Ends the output and thus releases the shared output stream.
     */
    ~SharedStringWriter();

private:
    ::util::stream::ISharedOutputStream& _stream;
};

} // namespace format
} // namespace util

#endif /* GUARD_205C0900_CB38_4D9D_BD89_0825E803775C */
