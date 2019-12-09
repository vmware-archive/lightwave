 /* Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an “AS IS” BASIS, without
 * warranties or conditions of any kind, EITHER EXPRESS OR IMPLIED.  See the
 * License for the specific language governing permissions and limitations
 * under the License.
 */

#ifndef _STRUCTS_H_
#define _STRUCTS_H_

typedef struct _REST_CLIENT_GLOBALS
{
    int bIsCurlInitialized;
    PSSO_CLIENT_CURL_INIT_CTX pCurlInitCtx;
} REST_CLIENT_GLOBALS, *PREST_CLIENT_GLOBALS;


#endif
