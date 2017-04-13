#if defined(WIN32)
# include <winsock2.h>
#endif
# include <bitmailcore/bitmail_internal.h>
# include <bitmailcore/bitmail.h>
# include <bitmailcore/email.h>
# include <bitmailcore/x509cert.h>
# include <bitmailcore/rpc.h>
# include <bitmailcore/multipart.h>

# include <curl/curl.h>
# include <openssl/cms.h>
# include <openssl/bio.h>
# include <openssl/err.h>
# include <openssl/bn.h>
# include <openssl/rsa.h>
# include <openssl/evp.h>
# include <openssl/x509.h>
# include <openssl/pem.h>
# include <openssl/rand.h>
# include <openssl/pkcs7.h>

# include <string>
# include <fstream>
# include <sstream>
# include <iostream>
# include <algorithm>

#include <string.h>

static void BitMailGlobalInit()
{
    OPENSSL_load_builtin_modules();
    ERR_load_crypto_strings();
    OpenSSL_add_all_ciphers();
    OpenSSL_add_all_algorithms();
    OpenSSL_add_all_digests();
    #ifdef WIN32
    WSAData wsa;
    WSAStartup(MAKEWORD(2,2), &wsa);
    #endif
}

BitMail::BitMail(ILockFactory * lock, IRTxFactory * net)
: m_onPollEvent(NULL), m_onPollEventParam(NULL)
, m_onMessageEvent(NULL), m_onMessageEventParam(NULL)
, m_mc(NULL)
, m_rx(NULL), m_tx(NULL)
, m_lock1(NULL), m_lock2(NULL), m_lock3(NULL), m_lock4(NULL)
{
    BitMailGlobalInit();

    if (lock){
        m_lock1 = lock->CreateLock();
        m_lock2 = lock->CreateLock();
        m_lock3 = lock->CreateLock();
        m_lock4 = lock->CreateLock();
        m_lockCraft = lock;
    }

    if (net){
        m_tx = net->CreateTx();
        m_rx = net->CreateRx();
        m_netCraft = net;
    }

    // Create a email object
    m_mc = new CMailClient(EmailHandler, this);

    // Create a profile object
    m_profile = new CX509Cert();
}

BitMail::~BitMail()
{
    if (m_profile != NULL){
        delete m_profile; m_profile = NULL;
    }
    if (m_mc != NULL){
        delete (m_mc); m_mc = NULL;
    }
    if (m_rx){
        m_netCraft->FreeRx(m_rx);
        m_rx = NULL;
    }
    if (m_tx){
        m_netCraft->FreeTx(m_tx);
        m_tx = NULL;
    }
    if (m_lock1){
        m_lockCraft->FreeLock(m_lock1);
        m_lock1 = NULL;
    }
    if (m_lock2){
        m_lockCraft->FreeLock(m_lock2);
        m_lock2 = NULL;
    }
    if (m_lock3){
        m_lockCraft->FreeLock(m_lock3);
        m_lock3 = NULL;
    }
    if (m_lock4){
        m_lockCraft->FreeLock(m_lock4);
        m_lock4 = NULL;
    }
}

unsigned int BitMail::GetVersion() const
{
    return    (BMVER_MAJOR * 1000000)
            + (BMVER_MINOR * 1000)
            + (BMVER_TINY);
}

/**
* Callback & params
*/
int BitMail::OnPollEvent( PollEventCB cb, void * userp)
{
    m_onPollEvent = cb;
    m_onPollEventParam = userp;
    return bmOk;
}

int BitMail::OnMessageEvent( MessageEventCB cb, void * userp)
{
    m_onMessageEvent = cb;
    m_onMessageEventParam = userp;
    return bmOk;
}

/**
* Network initialize
*/
int BitMail::InitNetwork( const std::string & txurl
                    , const std::string & txuser
                    , const std::string & txpass
                    , const std::string & rxurl
                    , const std::string & rxuser
                    , const std::string & rxpass)
{
    if (m_mc == NULL){
        return bmOutMem;
    }
    if (m_mc->InitTx(txurl, txuser, txpass)){
        return bmTxFail;
    }

    if (m_mc->InitRx(rxurl, rxuser, rxpass)){
        return bmRxFail;
    }
    return bmOk;
}

int BitMail::SetProxy(const std::string & ip
        , unsigned short port
        , const std::string & user
        , const std::string & password)
{
    return m_mc->SetProxy(ip, port, user, password);
}

void BitMail::SetProxyIp(const std::string & ip)
{
    m_mc->SetProxyIp(ip);
}

std::string BitMail::GetProxyIp() const
{
    return m_mc->GetProxyIp();
}

void BitMail::SetProxyPort(unsigned short port)
{
    m_mc->SetProxyPort(port);
}

unsigned short BitMail::GetProxyPort() const
{
    return m_mc->GetProxyPort();
}

void BitMail::SetProxyUser(const std::string & user)
{
    m_mc->SetProxyUser(user);
}

std::string BitMail::GetProxyUser() const
{
    return m_mc->GetProxyUser();
}

void BitMail::SetProxyPassword(const std::string & password)
{
    m_mc->SetProxyPassword(password);
}

std::string BitMail::GetProxyPassword() const
{
    return m_mc->GetProxyPassword();
}

int BitMail::SetTxUrl(const std::string & u)
{
    m_mc->SetTxUrl(u);
    return bmOk;
}

std::string BitMail::GetTxUrl() const
{
    return m_mc->GetTxUrl();
}

int BitMail::SetTxLogin(const std::string & l)
{
    m_mc->SetTxLogin(l);
    return bmOk;
}

std::string BitMail::GetTxLogin() const
{
    return m_mc->GetTxLogin();
}

int BitMail::SetTxPassword(const std::string & p)
{
    m_mc->SetTxPassword(p);
    return bmOk;
}

std::string BitMail::GetTxPassword() const
{
    return m_mc->GetTxPassword();
}

int BitMail::SetRxUrl(const std::string & u)
{
    m_mc->SetRxUrl(u);
    return bmOk;
}

std::string BitMail::GetRxUrl() const
{
    return m_mc->GetRxUrl();
}

int BitMail::SetRxLogin(const std::string & l)
{
    m_mc->SetRxLogin(l);
    return bmOk;
}

std::string BitMail::GetRxLogin() const
{
    return m_mc->GetRxLogin();
}

int BitMail::SetRxPassword(const std::string & p)
{
    m_mc->SetRxPassword(p);
    return bmOk;
}

std::string BitMail::GetRxPassword() const
{
    return m_mc->GetRxPassword();
}

int BitMail::SendMsg(const std::vector<std::string> & friends
        , const std::string & msg
        , RTxProgressCB cb, void * userp)
{
    std::string smime = EncMsg(friends, msg, false);

    if (m_mc->SendMsg( m_profile->GetEmail(), friends, smime, cb, userp))
    {
        return bmTxFail;
    }
    return bmOk;
}

int BitMail::CheckInbox(RTxProgressCB cb, void * userp)
{
    if (m_mc->CheckInbox(cb, userp)){
        return bmRxFail;
    }
    return bmOk;
}

int BitMail::StartIdle(unsigned int timeout, RTxProgressCB cb, void * userp)
{
    if (m_mc->StartIdle(timeout, cb, userp)){
        return bmIdleFail;
    }
    return bmOk;
}

int BitMail::Expunge(RTxProgressCB cb, void * userp)
{
    // Clear all <deleted> messages
    m_mc->Expunge(cb, userp);
}

/**
* Profile
*/
int BitMail::CreateProfile(const std::string & commonName
        , const std::string & email
        , const std::string & passphrase
        , unsigned int bits)
{
    if (NULL != m_profile)
    {
        delete m_profile;
        m_profile = NULL;
    }
    m_profile = new CX509Cert();
    if (m_profile == NULL){
        return bmOutMem;
    }
    if (m_profile->Create(commonName, email, passphrase, bits)){
        return bmNewProfile;
    }
    return bmOk;
}

int BitMail::LoadProfile(const std::string & passphrase
                         , const std::string & keyPem
                         , const std::string & certPem)
{
    m_profile = new CX509Cert();

    if (m_profile == NULL){
        return bmOutMem;
    }

    if (m_profile->LoadCertFromPem(certPem)){
        return bmInvalidCert;
    }

    if (m_profile->LoadKeyFromEncryptedPem(keyPem, passphrase)){
        return bmInvalidProfile;
    }

    return bmOk;
}

int BitMail::SaveProfile(const std::string & passphrase
                    , std::string & keypem
                    , std::string & certpem)
{
    return 0;
}

int BitMail::SetPassphrase(const std::string & passphrase)
{
    m_profile->SetPassphrase(passphrase);
    return bmOk;
}

std::string BitMail::GetNick() const
{
    return m_profile->GetCommonName();
}

std::string BitMail::GetID() const
{
    return m_profile->GetID();
}

std::string BitMail::GetEmail() const
{
    return m_profile->GetEmail();
}

std::string BitMail::GetCert() const
{
    return m_profile->GetCertByPem();
}

std::string BitMail::GetKey() const
{
    return m_profile->GetPrivateKeyAsEncryptedPem();
}

int BitMail::GetBits() const{
    return m_profile->GetBits();
}

std::string BitMail::GetPassphrase() const
{
    return m_profile->GetPassphrase();
}

std::string BitMail::Encrypt(const std::string & text) const
{
    return m_profile->Encrypt(text);
}

std::string BitMail::Decrypt(const std::string & code) const
{
    return m_profile->Decrypt(code);
}

int BitMail::splitMultiparts(const std::string & in, std::vector<std::string> & out)
{
    return multipart_split(in, out);
}

std::string BitMail::partType(const std::string & in)
{
    return multipart_partType(in);
}

std::string BitMail::partParam(const std::string & in, const std::string & header, const std::string & pName)
{
    return multipart_partParam(in, header, pName);
}

std::string BitMail::partContent(const std::string & in)
{
    return multipart_partContent(in);
}

std::string BitMail::partEncoding(const std::string & in)
{
    return multipart_partEncoding(in);
}

std::string BitMail::md5(const std::string & s)
{
    return CX509Cert::hash(s, "md5");
}

std::string BitMail::sha1(const std::string & s)
{
    return CX509Cert::hash(s, "sha1");
}

std::string BitMail::sha256(const std::string & s)
{
    return CX509Cert::hash(s, "sha256");
}

std::string BitMail::toBase64(const std::string & s)
{
    return CX509Cert::b64enc(s);
}

std::string BitMail::fromBase64(const std::string & s)
{
    return CX509Cert::b64dec(s);
}

std::string BitMail::EncMsg(const std::vector<std::string> & friends
        , const std::string & msg
        , bool fSignOnly)
{
    std::string smime;
    if (msg.empty()){
        return smime;
    }
    /**
     * Note:
     * GroupMsg(msg, vector<bob>) != SendMsg(msg, bob);
     * GroupMsg must encrypt msg before send it out.
     */
    smime = m_profile->Sign(msg);

    if (fSignOnly){
        return smime;
    }

    std::vector<CX509Cert> vecTo;
    for (std::vector<std::string>::const_iterator it = friends.begin();
            it != friends.end();
            ++it)
    {
        CX509Cert buddy;
        buddy.LoadCertFromPem(m_buddies[*it]);
        if (buddy.IsValid()){
            vecTo.push_back(buddy);
        }
    }

    if (!vecTo.size()){
        return smime;
    }

    smime = CX509Cert::MEncrypt(smime, vecTo);

    return smime;
}

int BitMail::DecMsg(const std::string & smime
        , std::string & from
        , std::string & nick
        , std::string & msg
        , std::string & certid
        , std::string & cert)
{
    std::string sMimeBody = smime;
    BitMail * self = this;
    if (CX509Cert::CheckMsgType(sMimeBody) == NID_pkcs7_enveloped){
        sMimeBody = self->m_profile->Decrypt(sMimeBody);
        if (sMimeBody.empty()){
            return bmDecryptFail;
        }
    }

    CX509Cert buddyCert;
    if (CX509Cert::CheckMsgType(sMimeBody) == NID_pkcs7_signed){
        buddyCert.LoadCertFromSig(sMimeBody);
        if (buddyCert.IsValid()){
            sMimeBody = buddyCert.Verify(sMimeBody);
            if (sMimeBody.empty()){
                return bmVerifyFail;
            }
        }else{
            return bmInvalidCert;
        }
    }else{
        return bmInvalidParam;
    }

    from = buddyCert.GetEmail();
    nick = buddyCert.GetCommonName();
    msg = sMimeBody;
    certid = buddyCert.GetID();
    cert = buddyCert.GetCertByPem();

    return bmOk;
}

std::string BitMail::GetFriendNick(const std::string & e) const
{
    if (e == GetEmail()){
        return m_profile->GetCommonName();
    }
    if (m_buddies.find(e) == m_buddies.end()){
        return "";
    }
    std::string sCert = m_buddies.find(e)->second;
    CX509Cert x;
    x.LoadCertFromPem(sCert);
    return x.GetCommonName();
}

std::string BitMail::GetFriendCert(const std::string & e) const
{
    if (e == GetEmail()){
        return m_profile->GetCertByPem();
    }
    if (m_buddies.find(e) == m_buddies.end()){
        return "";
    }
    std::string sCert = m_buddies.find(e)->second;
    return sCert;
}

std::string BitMail::GetFriendID(const std::string & e) const
{
    if (e == GetEmail()){
        return m_profile->GetID();
    }
    if (m_buddies.find(e) == m_buddies.end()){
        return "";
    }
    std::string sCert = m_buddies.find(e)->second;
    CX509Cert x;
    x.LoadCertFromPem(sCert);
    return x.GetID();
}

/**
* Friend APIs
*/
int BitMail::AddFriend(const std::string & email, const std::string &certpem)
{
    CX509Cert cert;
    cert.LoadCertFromPem(certpem);
    if (!cert.IsValid()){
        return bmInvalidCert;
    }
    std::string e = cert.GetEmail();
    if (email != e){
        return bmInvalidCert;
    }
    m_buddies[email] = certpem;
    return bmOk;
}

bool BitMail::HasFriend(const std::string & email) const
{
    if (m_buddies.find(email) != m_buddies.end()){
        return true;
    }
    return false;
}

bool BitMail::IsFriend(const std::string & email, const std::string & certpem) const
{
    CX509Cert testx;
    testx.LoadCertFromPem(certpem);
    if (!testx.IsValid()){
        return false;
    }
    std::string e = testx.GetEmail();
    if (e.empty() || email != e){
        return false;
    }
    if (m_buddies.find(e) == m_buddies.end()){
        return false;
    }
    std::string prevcert = m_buddies.find(e)->second;
    CX509Cert cert;
    cert.LoadCertFromPem(prevcert);
    if (!cert.IsValid()){
        return false;
    }
    return cert.GetEmail() == email
            && testx.GetID() == cert.GetID();
}


int BitMail::RemoveFriend(const std::string & email)
{
    if (m_buddies.find(email)!=m_buddies.end()){
        m_buddies.erase(email);
        return bmOk;
    }
    return bmNoBuddy;
}

int BitMail::GetFriends(std::vector<std::string> & vecEmails) const
{
    for (std::map<std::string, std::string>::const_iterator it = m_buddies.begin();
         it != m_buddies.end();
         ++it){
         vecEmails.push_back(it->first);
    }
    return bmOk;
}

// Groups
int BitMail::GetGroups(std::vector<std::string> & groups) const
{
    for (std::set<std::string>::const_iterator it = m_groups.begin(); it != m_groups.end(); ++it){
        groups.push_back(*it);
    }
    return bmOk;
}

int BitMail::AddGroup(const std::string & group)
{
    m_groups.insert(group);
    return bmOk;
}

int BitMail::RemoveGroup(const std::string & group)
{
    if (m_groups.find( (group) ) != m_groups.end()){
        m_groups.erase(m_groups.find( ( group )));
    }
    return bmOk;
}

// Receips
std::vector<std::string> BitMail::decodeReceips(const std::string & receips)
{
    std::istringstream iss(receips);
    std::vector<std::string> v;
    std::string line;
    while (std::getline(iss, line, ';')){
        if (line.empty()) continue;
        if (line.find('@') != std::string::npos){
            std::transform(line.begin(), line.end(), line.begin(), ::tolower);
            v.push_back(line);
        }
    }
    std::sort(v.begin(), v.end());
    std::unique(v.begin(), v.end());
    return v;
}

std::string BitMail::serializeReceips(const std::vector<std::string> & vec_receips)
{
    std::string receips;
    for (std::vector<std::string>::const_iterator it = vec_receips.begin(); it != vec_receips.end(); ++it){
        receips += *it;
        receips += ";";
    }
    return receips;
}

// Session Key
std::string BitMail::toSessionKey(const std::string & receip)
{
    std::vector<std::string> v;
    v.push_back(receip);
    return toSessionKey(v);
}

std::string BitMail::toSessionKey(const std::vector<std::string> & vec_receips)
{
    return toBase64( serializeReceips(vec_receips) );
}

std::vector<std::string> BitMail::fromSessionKey(const std::string & sessKey)
{
    return decodeReceips( fromBase64(sessKey) );
}

// Mime parser
int BitMail::EmailHandler(BMEventHead * h, void * userp)
{
    BitMail * self = (BitMail *)userp;

    if (h->magic != BMMAGIC){
        return bmInvalidParam;
    }

    if (h->bmef == bmefMsgCount){
        if (self && self->m_onPollEvent){
            return self->m_onPollEvent(((BMEventMsgCount*)h)->msgcount
                                        , self->m_onPollEventParam);
        }else{
            return bmInvalidParam;
        }
    }else if (h->bmef == bmefSystem){
        return 0;
    }else if (h->bmef == bmefMessage){
    }else{
        return bmInvalidParam;
    }

    BMEventMessage * bmeMsg = (BMEventMessage *)h;

    std::string sMimeBody = bmeMsg->msg;;

    std::string receips = parseRFC822AddressList(sMimeBody);

    /**
    * Crypto filter
    */
    if (CX509Cert::CheckMsgType(sMimeBody) == NID_pkcs7_enveloped){
        sMimeBody = self->m_profile->Decrypt(sMimeBody);
        if (sMimeBody.empty()){
            return bmDecryptFail;
        }
    }

    CX509Cert buddyCert;
    if (CX509Cert::CheckMsgType(sMimeBody) == NID_pkcs7_signed){
        buddyCert.LoadCertFromSig(sMimeBody);
        if (buddyCert.IsValid()){
            sMimeBody = buddyCert.Verify(sMimeBody);
            if (sMimeBody.empty()){
                return bmVerifyFail;
            }
        }else{
            return bmInvalidCert;
        }
    }

    if (self && self->m_onMessageEvent){
        // TODO: parse all receips from header `To'.
        self->m_onMessageEvent(buddyCert.GetEmail().c_str()
                                , receips.c_str()
                                , sMimeBody.data()
                                , sMimeBody.length()
                                , buddyCert.GetID().c_str()
                                , buddyCert.GetCertByPem().c_str()
                                , self->m_onMessageEventParam);
    }

    return bmOk;
}

std::string BitMail::parseRFC822AddressList(const std::string & mimemsg)
{
    /**
      *TODO: parse address list
      * https://www.w3.org/Protocols/rfc822/#z8
      * https://tools.ietf.org/html/rfc5322#section-2.2.3
      * TODO: Split lines by LF now, but should be CRLF
    */
    std::string receips;
    do {
        std::istringstream iss(mimemsg);
        std::string line;
        while (std::getline(iss, line))
        {
            // clear CR
            while(line.find('\r') != std::string::npos){
                line.erase(line.find('\r'));
            }

            const char * p = line.c_str();

            // initial state
            if (receips.empty()){
                if (*p && (*p == 't' || *p == 'T')) p++;
                else continue;
                if (*p && (*p == 'o' || *p == 'O')) p++;
                else continue;
                if (*p && *p == ':') p++;
                else continue;
                receips = line;
            }else{
            // mulit-line mode: folding
                if (*p && !::isspace(*p)) break;
                else{
                    receips += line;
                }
            }
        }
    }while(0);


    // a weak RFC822 address parser
    do{
        if (receips.empty()) break;

        char * receips_buf = strdup(receips.c_str());
        receips.clear();

        if (!receips_buf) break;

        const char * delims = "~`!$%^&*-+=\\{}[]?<>()\"':,;|/ \t\r\n";
        const char * tok = strtok(receips_buf, delims);
        while(tok){
            if (*tok && strchr(tok, '@')){
                receips += tok;
                receips += ";";
            }
            tok = strtok(NULL, delims);
        }

        free(receips_buf);
    }while(0);

    return (receips);
}
