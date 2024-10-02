// Copyright 2024 Accenture.

#ifndef GUARD_5D29F5A7_99BB_4545_AFCE_4EA1970286F1
#define GUARD_5D29F5A7_99BB_4545_AFCE_4EA1970286F1

#include "platform/estdint.h"

namespace uds
{
/**
 * Container for common diagnosis return values.
 *
 *
 * \see ISO 14229
 */
class DiagReturnCode
{
    DiagReturnCode();

public:
    /** first byte of a diagnosis negative response */
    static uint8_t const NEGATIVE_RESPONSE_IDENTIFIER = 0x7FU;

    /**
     * offset added to the service identifier of a request in case of a
     * positive response.
     *
     * \section example Example
     * request: 0x22,0xXX,0xXX (read data by identifier)
     * response: 0x62,... (positive response)
     */
    static uint8_t const POSITIVE_RESPONSE_OFFSET = 0x40U;

    /**
     * Return codes used throughout diagnosis framework.
     *
     * \note * The prefix ISO_ indicates that this is an official ISO14229 code.
     *
     * @important
     * The DiagReturnCode::Type NOT_RESPONSIBLE has a special meaning. Returning
     * any other value from verify indicates that the job has taken control
     * over the request and will handle it. No further job will be notified
     * of the request in question!
     */
    enum Type
    {
        ISO_GENERAL_REJECT                                         = 0x10,
        ISO_SERVICE_NOT_SUPPORTED                                  = 0x11,
        ISO_SUBFUNCTION_NOT_SUPPORTED                              = 0x12,
        ISO_INVALID_FORMAT                                         = 0x13,
        ISO_RESPONSE_TOO_LONG                                      = 0x14,
        ISO_BUSY_REPEAT_REQUEST                                    = 0x21,
        ISO_CONDITIONS_NOT_CORRECT                                 = 0x22,
        ISO_REQUEST_SEQUENCE_ERROR                                 = 0x24,
        ISO_CONTROL_UNIT_ON_SUBBUS_NOT_RESPONDING                  = 0x25,
        ISO_REQUEST_OUT_OF_RANGE                                   = 0x31,
        ISO_SECURITY_ACCESS_DENIED                                 = 0x33,
        ISO_AUTHENTICATION_REQUIRED                                = 0x34,
        ISO_INVALID_KEY                                            = 0x35,
        ISO_EXCEEDED_NUMS_OF_ATTEMPTS                              = 0x36,
        ISO_REQUIRED_TIME_DELAY_NOT_EXPIRED                        = 0x37,
        ISO_CERTIFICATE_VERIFICATION_FAILED_INVALID_TIME_PERIOD    = 0x50,
        ISO_CERTIFICATE_VERIFICATION_FAILED_INVALID_SIGNATURE      = 0x51,
        ISO_CERTIFICATE_VERIFICATION_FAILED_INVALID_CHAIN_OF_TRUST = 0x52,
        ISO_CERTIFICATE_VERIFICATION_FAILED_INVALID_TYPE           = 0x53,
        ISO_CERTIFICATE_VERIFICATION_FAILED_INVALID_FORMAT         = 0x54,
        ISO_CERTIFICATE_VERIFICATION_FAILED_INVALID_CONTENT        = 0x55,
        ISO_CERTIFICATE_VERIFICATION_FAILED_INVALID_SCOPE          = 0x56,
        ISO_CERTIFICATE_VERIFICATION_FAILED_CERTIFICATE_REVOKED    = 0x57,
        ISO_OWNERSHIP_VERIFICATION_FAILED                          = 0x58,
        ISO_SETTING_ACCESS_RIGHTS_FAILED                           = 0x5A,
        ISO_UPLOAD_DOWNLOAD_NOT_ACCEPTED                           = 0x70,
        ISO_TRANSFER_DATA_SUSPENDED                                = 0x71,
        ISO_GENERAL_PROGRAMMING_FAILURE                            = 0x72,
        ISO_WRONG_BLOCK_SEQUENCE_COUNTER                           = 0x73,
        ISO_RESPONSE_PENDING                                       = 0x78,
        ISO_SUBFUNCTION_NOT_SUPPORTED_IN_ACTIVE_SESSION            = 0x7E,
        ISO_SERVICE_NOT_SUPPORTED_IN_ACTIVE_SESSION                = 0x7F,
        /** The job verifying a request is not responsible for handling it */
        NOT_RESPONSIBLE                                            = 0xFE,
        /** The job wants to handle the request */
        OK                                                         = 0xFF
    };
};

} // namespace uds

#endif // GUARD_5D29F5A7_99BB_4545_AFCE_4EA1970286F1
