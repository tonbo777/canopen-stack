/******************************************************************************
   Copyright 2020 Embedded Office GmbH & Co. KG

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.
******************************************************************************/

#ifndef CO_CSDO_H_
#define CO_CSDO_H_

#if defined __cplusplus
extern "C" {
#endif

/******************************************************************************
* INCLUDES
******************************************************************************/

#include "co_types.h"
#include "co_sdo.h"
#include "co_if.h"

/******************************************************************************
* PUBLIC TYPES
******************************************************************************/

/*! \brief SDO CLIENT STATE
 *
 *   This enumerations holds the possible SDO client transfer states.
 *   It is mainly used for deciding if next user request is possible.
 *
 */
typedef enum {
    CO_CSDO_STATE_INVALID = 0,       /*!< SDO client is not enabled          */
    CO_CSDO_STATE_IDLE    = 1,       /*!< SDO client is idle                 */
    CO_CSDO_STATE_BUSY    = 2,       /*!< SDO client handling transfer       */

} CO_CSDO_STATE;

/*! \brief SDO CLIENT TRANSFER TYPE
 *
 *   This enumeration holds the possible SDO client transfer types.
 *   Currently, only expedited transfer is supported, it might be
 *   required to extend this enumeration in future in case segmented
 *   and block transfer support will be added.
 */
typedef enum {
    CO_CSDO_TRANSFER_NONE     = 0,   /*!< No transfer is currently ongoing   */
    CO_CSDO_TRANSFER_UPLOAD   = 1,   /*!< SDO upload is being executed       */
    CO_CSDO_TRANSFER_DOWNLOAD = 2,   /*!< SDO download is being executed     */
    CO_CSDO_TRANSFER_UPLOAD_SEGMENT = 3,  /*!< SDO segment upload is being executed     */
    CO_CSDO_TRANSFER_DOWNLOAD_SEGMENT = 4, /*!< SDO segment download is being executed     */

} CO_CSDO_TRANSFER_TYPE;

struct CO_CSDO_T;

/*! \brief
 *
 *   Notification on SDO client transfer complete operation.
 *
 * \param csdo
 *   Reference to SDO client that initiated transfer
 *
 * \param index
 *   Dictionary index
 *
 * \param sub
 *   Dictionary subindex
 *
 * \param code
 *   Abort code of transfer (=0 if transfer was successful)
 *
 */
typedef void (*CO_CSDO_CALLBACK_T)(struct CO_CSDO_T *csdo,
                                   uint16_t          index,
                                   uint8_t           sub,
                                   uint32_t          code);


/*! \brief SDO SEGMENTED TRANSFER
*
*    This structure holds the data, which are needed for the segmented
*    SDO transfer.
*/
typedef struct CO_CSDO_SEG_T {
    uint32_t  Size;              /*!< Size of object entry                 */
    uint32_t  Num;               /*!< Number of transfered bytes            */
    uint8_t   TBit;              /*!< Segment toggle bit                    */

} CO_CSDO_SEG;



/*! \brief SDO CLIENT TRANSFER
 *
 *   This structure contains information required for handling ongoing SDO
 *   transfer requested by this node.
 *
 */
typedef struct CO_CSDO_TRANSFER_T {
    struct CO_CSDO_T      *Csdo;        /*!< Reference to responsible CSDO   */
    CO_CSDO_TRANSFER_TYPE  Type;        /*!< Type of current transfer        */
    uint32_t               Abort;       /*!< Abort code of transfer          */
    uint16_t               Idx;         /*!< Accessed dictionary index       */
    uint8_t                Sub;         /*!< Accessed dictionary subindex    */
    uint32_t               Tmt;         /*!< Timeout value in milliseconds   */
    uint8_t               *Buf;         /*!< Reference to transfered data    */
    uint32_t               Size;        /*!< Transfered data size            */
    int16_t                Tmr;         /*!< Identifier of timeout timer     */
    CO_CSDO_CALLBACK_T     Call;        /*!< Notification callback           */
    uint32_t               Buf_Idx;     /*!< Buffer Index                    */
    uint8_t                TBit;        /*!< Segment toggle bit              */
} CO_CSDO_TRANSFER;

/*! \brief SDO CLIENT
 *
 *   This structure contains information required for SDO client
 *   functionality implementation.
 */
typedef struct CO_CSDO_T {
    struct CO_NODE_T *Node;             /*!< Link to node info structure     */
    CO_IF_FRM        *Frm;              /*!< SDO req/res CAN frame           */
    uint32_t          RxId;             /*!< Identifier for CSDO response    */
    uint32_t          TxId;             /*!< Identifier for CSDO requests    */
    uint8_t           NodeId;           /*!< Node-Id of addressed SDO server */
    CO_CSDO_STATE     State;            /*!< Current CSDO state              */
    CO_CSDO_TRANSFER  Tfer;             /*!< Current CSDO transfer info      */
} CO_CSDO;

/******************************************************************************
* PUBLIC FUNCTIONS
******************************************************************************/

/*! \brief
 *
 *   This function checks if SDO client structure number 'num' exists and is
 *   enabled. If valid handle is found, its reference is returned.
 *
 * \param node
 *   Reference to parent CANopen node
 *
 * \param num
 *   Index of SDO client
 *
 * \retval  >0  valid SDO client has been found
 * \retval  =0  SDO client 'num' is not valid or does not exist
 *
 */
CO_CSDO *COCSdoFind(struct CO_NODE_T *node, uint8_t num);

/*! \brief
 *
 *   This function initiates SDO upload sequence. User should provide its
 *   own notification callback which is executed upon transfer completion.
 *
 * \param csdo
 *   Reference to SDO client
 *
 * \param key
 *    object entry key; should be generated with the macro CO_DEV()
 *
 * \param buf
 *   Reference to data to be downloaded to server
 *
 * \param size
 *   Size of downloaded data
 *
 * \param callback
 *   Notification callback on tranfer end (complete or abort)
 *
 * \param
 *   SDO server response timeout in milliseconds
 *
 * \retval  ==CO_ERR_NONE   transfer initiated successfuly
 * \retval  !=CO_ERR_NONE   SDO client is busy or invalid
 *
 */
CO_ERR COCSdoRequestUpload(CO_CSDO *csdo,
                           uint32_t key,
                           uint8_t *buf,
                           uint32_t size,
                           CO_CSDO_CALLBACK_T callback,
                           uint32_t timeout);

/*! \brief
 *
 *   This function initiates SDO download sequence. User should provide its
 *   own notification callback which is executed upon transfer completion.
 *
 * \param csdo
 *   Reference to SDO client
 *
 * \param key
 *    object entry key; should be generated with the macro CO_DEV()
 *
 * \param buf
 *   Reference to data to be downloaded to server
 *
 * \param size
 *   Size of downloaded data
 *
 * \param callback
 *   Notification callback on tranfer end (complete or abort)
 *
 * \param timeout
 *   SDO server response timeout in milliseconds
 *
 * \retval  ==CO_ERR_NONE   transfer initiated successfuly
 * \retval  !=CO_ERR_NONE   SDO client is busy or invalid
 *
 */
CO_ERR COCSdoRequestDownload(CO_CSDO *csdo,
                             uint32_t key,
                             uint8_t *buf,
                             uint32_t size,
                             CO_CSDO_CALLBACK_T callback,
                             uint32_t timeout);

/******************************************************************************
* PRIVATE FUNCTIONS
******************************************************************************/

/*! \brief
 *
 *   This function reads the content of the object dictionary with
 *   index 1280h+[n] subindex 1 (for TX) and subindex 2 (for RX),
 *   where n is a counter from 0 to maximal number of supported SDO
 *   clients specified by user or in `co_config.h` (CO_CSDO_N).
 *
 * \param csdo
 *   Reference to SDO client
 *
 * \param index
 *   Reference to parent CANopen node
 *
 */
void COCSdoInit(CO_CSDO *csdo, struct CO_NODE_T *node);

/*! \brief
 *
 *   This function resets all internals of the given SDO client 'num'.
 *
 * \param csdo
 *   Reference to SDO client
 *
 * \param num
 *   Index of SDO client
 *
 * \param index
 *   Reference to parent CANopen node
 *
 */
void COCSdoReset(CO_CSDO *csdo, uint8_t num, struct CO_NODE_T *node);

/*! \brief
 *
 *   This function reads the content of the object dictionary for a
 *   given SDO client number 'num' from the with index 1280h+[num]
 *   subindex 1 (for TX) and subindex 2 (for RX). The identifiers
 *   for request and response will be set acc. the CANopen rules.
 *   If everything is ok, the SDO client is enabled after this
 *   function call.
 *
 * \param csdo
 *   Reference to SDO client
 *
 * \param num
 *   Index of SDO client
 *
 */
void COCSdoEnable(CO_CSDO *csdo, uint8_t num);

/*! \brief  CHECK FOR RESPONSE TO SDO CLIENT
*
*    This function checks the given frame to be a response to SDO
*    client request. If the frame is identified to be a valid
*    response, it is processed and in case response is required
*    (segmented or block transfer), frame is modified to be the
*    corresponding SDO response.
*
* \param csdo
*    Ptr to root element of SDO client array
*
* \param frm
*    Frame, received from CAN bus
*
* \retval  >0    pointer to addressed SDO client
* \retval  =0    not a SDO request or error is detected
*/
CO_CSDO *COCSdoCheck(CO_CSDO *csdo, CO_IF_FRM *frm);

/*! \brief  GENERATE SDO CLIENT RESPONSE
*
*    This function interprets the data byte #0 of the response
*    from addressed SDO server request and organizes the
*    generation of the corresponding response.
*
* \param csdo
*    Ptr to addressed SDO client
*
* \retval  ==CO_ERR_NONE        transmit SDO client frame
* \retval  ==CO_ERR_SDO_ABORT   transmit SDO abort frame
* \retval  ==CO_ERR_SDO_SILENT  transfer complete, do not transmit
*/
CO_ERR COCSdoResponse(CO_CSDO *csdo);

/*! \brief  SDO CLIENT EXPEDITED UPLOAD PROTOCOL
*
*    This function processes response from SDO client to expedited
*    upload request.
*
* \param csdo
*    Ptr to addressed SDO client
*
* \retval  ==CO_ERR_NONE    successful transfer
* \retval  !=CO_ERR_NONE    an error is detected
*/
CO_ERR COCSdoUploadExpedited(CO_CSDO *csdo);

/*! \brief  SDO CLIENT EXPEDITED DOWNLOAD PROTOCOL
*
*    This function processes response from SDO client to expedited
*    download request.
*
* \param csdo
*    Ptr to addressed SDO client
*
* \retval  ==CO_ERR_NONE    successful transfer
* \retval  !=CO_ERR_NONE    an error is detected
*/
CO_ERR COCSdoDownloadExpedited(CO_CSDO *csdo);

/*! \brief  SDO CLIENT PROTOCOL TIMEOUT
*
*    Notification executed when waiting for SDO response from server
*    has timed out.
*
* \param parg
*    Reference to concerning SDO client
*/
void COCSdoTimeout(void *parg);

/*! \brief  ABORT PROTOCOL
*
*    This function generates the abort frame from addressed SDO client.
*
* \param csdo
*    Pointer to SDO client object
*
* \param err
*    The error code to be inserted in the frame data [7:4]
*/
void COCSdoAbort(CO_CSDO *csdo, uint32_t err);

/*! \brief  FINALIZE SDO CLIENT TRANSFER
*
*    This function handles SDO client request finalization, execution
*    of user notifications and releasing SDO client context for next
*    transfer requests.
*
* \param csdo
*    Pointer to SDO client object
*/
void COCSdoTransferFinalize(CO_CSDO *csdo);

/*! \brief  INIT CSDO SEGMENTED UPLOAD
*
*    This function generates the request for 'First Upload SDO Segment Protocol'.
*
*    Entry condition for this function is the SDO response command byte
*    with the following condition:
* \code
*    +---+---+---+---+---+---+---+---+
*    |     2     | 0 |   0   | 0 | 1 |
*    +---+---+---+---+---+---+---+---+
*      7   6   5   4   3   2   1   0
*
*    => condition := "command must be 0x41"*
* \endcode
*
* \param csdo
*    Pointer to SDO client object
*
* \param width
*    Object size in byte
*
* \retval  ==CO_ERR_NONE    on success
* \retval  !=CO_ERR_NONE    on CSDO abort
*/
CO_ERR COCSdoInitUploadSegmented(CO_CSDO *csdo);

/*! \brief  SEGMENTED UPLOAD
*
*    This function generates the request for 'Upload SDO Segment Protocol'.
*
*    Entry condition for this function is the SDO response command byte
*    with the following condition:
* \code
*    +---+---+---+---+---+---+---+---+
*    |     0     | t |     n     | c |
*    +---+---+---+---+---+---+---+---+
*      7   6   5   4   3   2   1   0
*
*    => condition = "command & 0xE0 must be 0x00"
* \endcode
*
* \param csdo
*    Pointer to SDO client object
*
* \retval  ==CO_ERR_NONE    on success
* \retval  !=CO_ERR_NONE    on CSDO abort
*/
CO_ERR COCSdoUploadSegmented(CO_CSDO *csdo);

/*! \brief 
 *
 *   This function initiates SDO segmented upload sequence. User should provide its
 *   own notification callback which is executed upon transfer completion.
 *
 * \param csdo
 *   Reference to SDO client
 *
 * \param key
 *    object entry key; should be generated with the macro CO_DEV()
 *
 * \param buf
 *   Reference to buffer to be uploaded from server
 *
 * \param size
 *   Size of uploaded buffer
 *
 * \param callback
 *   Notification callback on tranfer end (complete or abort)
 *
 * \param
 *   SDO server response timeout in milliseconds
 *
 * \retval  ==CO_ERR_NONE   transfer initiated successfuly
 * \retval  !=CO_ERR_NONE   SDO client is busy or invalid
 *
 */
CO_ERR COCSdoRequestSegmentUpload(CO_CSDO *csdo,
                                uint32_t key,
                                uint8_t *buf,
                                uint32_t size,
                                CO_CSDO_CALLBACK_T callback,
                                uint32_t timeout);

/*! \brief  INIT CSDO SEGMENTED DOWNLOAD
*
*    This function generates the request for 'First Download SDO Segment
*    Protocol'.
*
*    Entry condition for this function is the SDO response command byte
*    with the following condition:
* \code
*    +---+---+---+---+---+---+---+---+
*    |     3     | 0   0   0   0   0 |
*    +---+---+---+---+---+---+---+---+
*      7   6   5   4   3   2   1   0
*
*    => condition = "command must be 0x60"
* \endcode
*
* \param csdo
*    Pointer to SDO client object
*
* \retval  ==CO_ERR_NONE    on success
* \retval  !=CO_ERR_NONE    on CSDO abort
*/
CO_ERR COCSdoInitDownloadSegmented(CO_CSDO *csdo);

/*! \brief  SEGMENTED DOWNLOAD
*
*    This function generates the request for 'Download SDO Segment
*    Protocol'.
*
*    Entry condition for this function is the SDO request command byte
*    with the following condition:
* \code
*    +---+---+---+---+---+---+---+---+
*    |     0     | t |     n     | c |
*    +---+---+---+---+---+---+---+---+
*      7   6   5   4   3   2   1   0
*
*    => condition = "command & 0xE0 must be 0x00"
* \endcode
*
* \param csdo
*    Pointer to SDO client object
*
* \retval  ==CO_ERR_NONE    on success
* \retval  !=CO_ERR_NONE    on CSDO abort
*/
CO_ERR COCSdoDownloadSegmented(CO_CSDO *csdo);

/*! \brief
 *
 *   This function finish SDO segmented download sequence.
 *  
 * \param csdo
 *   Reference to SDO client
 *
 */
CO_ERR COCSdoFinishDownloadSegmented(CO_CSDO *csdo);

/*! \brief
 *
 *   This function initiates SDO segmented download sequence. User should provide its
 *   own notification callback which is executed upon transfer completion.
 *
 * \param csdo
 *   Reference to SDO client
 *
 * \param key
 *    object entry key; should be generated with the macro CO_DEV()
 *
 * \param buf
 *   Reference to data to be downloaded to server
 *
 * \param size
 *   Size of downloaded data
 *
 * \param callback
 *   Notification callback on tranfer end (complete or abort)
 *
 * \param timeout
 *   SDO server response timeout in milliseconds
 *
 * \retval  ==CO_ERR_NONE   transfer initiated successfuly
 * \retval  !=CO_ERR_NONE   SDO client is busy or invalid
 *
 */
CO_ERR COCSdoRequestSegmentDownload(CO_CSDO *csdo,
                                uint32_t key,
                                uint8_t *buf,
                                uint32_t size,
                                CO_CSDO_CALLBACK_T callback,
                                uint32_t timeout);



#if defined __cplusplus
}
#endif

#endif /* CO_CSDO_H_ */
