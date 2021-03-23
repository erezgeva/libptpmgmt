/* SPDX-License-Identifier: GPL-3.0-or-later */

/** @file
 * @brief Impleament linuxptp pmc tool using the libpmc library
 *
 * @author Erez Geva <ErezGeva2@@gmail.com>
 * @copyright 2021 Erez Geva
 *
 * Created following "IEEE Std 1588-2008", PTP version 2
 */

#include <signal.h>
#include <unistd.h>
#include "msg.h"
#include "ptp.h"
#include "sock.h"
#include "bin.h"

/* from pmc_dump.cpp */
extern void call_dump(message &msg);
extern baseData *call_data(message &msg, mng_vals_e id, char *save);

/* Receive constants */
static const int wait = 500; // milli

static const char toksep[] = " \t\n\r"; // while spaces
// buffer for send and recv
static const size_t bufSize = 2000;
static char buf[bufSize];
static message msg;
static std::unique_ptr<sockBase> sk;
static binary clockIdentity;
static bool use_uds;
static uint64_t timeout;

static inline const char *act2str()
{
    switch(msg.getAction()) {
        case GET:
            return "GET";
        case SET:
            return "SET";
        default:
            return "COMMAND";
    }
}
static inline void dump_head()
{
    printf("\t%s seq %u RESPONSE MANAGEMENT %s ",
        msg.getPeer().str().c_str(),
        msg.getSequence(),
        msg.mng2str_c(msg.getTlvId()));
}
static inline void dump_err()
{
    printf("\t%s seq %u RESPONSE MANAGEMENT_ERROR_STATUS %s\n"
        "\tERROR: %s(%u)\n"
        "\tERROR DISPLAY: %s\n\n",
        msg.getPeer().str().c_str(),
        msg.getSequence(),
        msg.mng2str_c(msg.getTlvId()),
        msg.errId2str_c(msg.getErrId()),
        msg.getErrId(),
        msg.getErrDisplay_c());
}
static inline bool updatePortIdentity(msgParams &prms, char *str)
{
    char *port = strchr(str, '-');
    if(port == nullptr)
        return false;
    *port = 0;
    binary nc;
    if(!nc.fromHex(str) || nc.length() != ClockIdentity_t::size())
        return false;
    port++;
    char *end;
    long val = strtol(port, &end, 0);
    if(*end != 0 || val < 0 || val > UINT16_MAX)
        return false;
    prms.target.portNumber = val;
    nc.copy(prms.target.clockIdentity.v);
    return true;
}
static inline bool sendAction()
{
    static int seq = 0;
    MNG_PARSE_ERROR_e err = msg.build(buf, bufSize, seq);
    if(err != MNG_PARSE_ERROR_OK) {
        printf("build error %s\n", msg.err2str_c(err));
        return false;
    }
    if(!sk->send(buf, msg.getMsgLen()))
        return false;
    seq++;
    return true;
}
static inline int rcv()
{
    auto cnt = sk->rcv(buf, bufSize);
    if(cnt < 0)
        return -1;
    MNG_PARSE_ERROR_e err = msg.parse(buf, cnt);
    switch(err) {
        case MNG_PARSE_ERROR_MSG:
            dump_err();
            break;
        case MNG_PARSE_ERROR_OK:
            dump_head();
            call_dump(msg);
            return 0;
        case MNG_PARSE_ERROR_ACTION: // Not managment, or other clock id
        case MNG_PARSE_ERROR_HEADER: // Not reply
            if(!use_uds)
                // We got the wrong message, wait for the next one
                return 1;
        default:
            printf("Parse error %s\n", msg.err2str_c(err));
            break;
    }
    return -1;
}
static inline void rcv_timeout()
{
    timeout = wait;
    while(sk->tpoll(timeout) && rcv() == 1 && timeout > 0);
}
static inline bool findId(mng_vals_e &id, char *str)
{
    size_t len = strlen(str);
    if(len == 0)
        return false;
    // Make string uppercase as all commands are uppercase
    for(char *cur = str; *cur; cur++)
        *cur = toupper(*cur);
    if(strstr(str, "NULL") != nullptr) {
        id = NULL_PTP_MANAGEMENT;
        return true;
    }
    int find = 0;
    for(int i = FIRST_MNG_ID; i <= LAST_MNG_ID; i++) {
        const char *sid = message::mng2str_c((mng_vals_e)i);
        if(strcmp(sid, str) == 0) {
            id = (mng_vals_e)i;
            return true; // Exect match!
        }
        if(find < 2 && strncmp(sid, str, len) == 0) {
            id = (mng_vals_e)i;
            find++;
        }
    }
    // 1 matach
    return find == 1;
}
static bool run_line(char *line)
{
    char *save;
    char *cur = strtok_r(line, toksep, &save);
    if(cur == nullptr)
        return false;
    actionField_e action;
    if(strcasecmp(cur, "get") == 0)
        action = GET;
    else if(strcasecmp(cur, "set") == 0)
        action = SET;
    else if(strcasecmp(cur, "cmd") == 0 || strcasecmp(cur, "command") == 0)
        action = COMMAND;
    else if(strcasecmp(cur, "target") == 0) {
        cur = strtok_r(nullptr, toksep, &save);
        if(cur == nullptr || *cur == 0)
            return false;
        if(*cur == '*')
            msg.setAllClocks();
        else {
            msgParams prms = msg.getParams();
            if(updatePortIdentity(prms, cur))
                msg.updateParams(prms);
            else
                fprintf(stderr, "Wrong clock ID: %s\n", cur);
        }
        return false;
    } else
        return false;
    cur = strtok_r(nullptr, toksep, &save);
    if(cur == nullptr)
        return false;
    mng_vals_e id;
    if(!findId(id, cur))
        return false;
    baseData *data;
    if(action == GET || msg.isEmpty(id)) {
        // No data is needed
        if(!msg.setAction(action, id))
            return false;
        data = nullptr;
    } else {
        data = call_data(msg, id, save);
        // No point to send without data
        if(data == nullptr)
            return false;
        if(!msg.setAction(action, id, *data))
            return false;
    }
    if(!sendAction())
        return false;
    // Finish with data, free it
    if(data != nullptr)
        delete data;
    printf("sending: %s %s\n", act2str(), msg.mng2str_c(id));
    return true;
}
static void handle_sig(int)
{
    sk->close();
    exit(0);
}
static void handle_sig_ctrl(int)
{
    sk->close();
    printf("\n");
    exit(0);
}
int main(int argc, char *const argv[])
{
    int c;
    std::map<int, std::string> options;
    const char *with_options = "f:b:d:s:t:i:v";
    const char *net_options = "u246";
    // Always send GET with zero length data.
    const char *ignore_options = "z";
    std::string opts = with_options;
    opts += net_options;
    opts += ignore_options;
    char net_select = 0;
    while((c = getopt(argc, argv, opts.c_str())) != -1) {
        switch(c) {
            case ':':
                printf("Wrong option ':'\n");
                return -1;
            case 'v':
                printf("%s\n", message::getVersion());
                return 0;
            default:
                break;
        }
        if(strrchr(with_options, c) != nullptr)
            options[c] = optarg;
        else if(strrchr(net_options, c) != nullptr)
            net_select = c;
        else if(strrchr(ignore_options, c) == nullptr) {
            printf("Wrong option '%c:'\n", c);
            return -1;
        }
    }
    configFile cfg;
    /* handle configuration file */
    if(options.count('f') && !cfg.read_cfg(options['f']))
        return -1;
    if(net_select == 0)
        net_select = cfg.network_transport();
    std::string interface;
    if(options.count('i') && !options['i'].empty())
        interface = options['i'];
    ifInfo ifObj;
    msgParams prms = msg.getParams();
    if(net_select != 'u') {
        if(interface.empty()) {
            fprintf(stderr, "missing interface\n");
            return -1;
        }
        if(!ifObj.initUsingName(interface))
            return -1;
        clockIdentity = ifObj.mac();
        clockIdentity.eui48ToEui64();
        clockIdentity.copy(prms.self_id.clockIdentity.v);
        prms.self_id.portNumber = 1;
        use_uds = false;
    }
    if(options.count('b'))
        prms.boundaryHops = atoi(options['b'].c_str());
    else
        prms.boundaryHops = 1;
    if(options.count('d'))
        prms.domainNumber = atoi(options['b'].c_str());
    else
        prms.domainNumber = cfg.domainNumber(interface);
    if(options.count('t'))
        prms.transportSpecific = strtol(options['t'].c_str(), nullptr, 16);
    else
        prms.transportSpecific = cfg.transportSpecific(interface);
    switch(net_select) {
        case 'u': {
            sockUnix *sku = new sockUnix;
            if(sku == nullptr) {
                fprintf(stderr, "failed to allocate sockUnix\n");
                return -1;
            }
            std::string uds_address;
            if(options.count('s'))
                uds_address = options['s'];
            else
                uds_address = cfg.uds_address(interface);
            if(!sku->setDefSelfAddress() || !sku->init() ||
                !sku->setPeerAddress(uds_address)) {
                fprintf(stderr, "failed to create transport\n");
                return -1;
            }
            sk = std::move(std::unique_ptr<sockBase>(sku));
            prms.self_id.portNumber = getpid();
            use_uds = true;
            break;
        }
        default:
        case '4': {
            sockIp4 *sk4 = new sockIp4;
            if(sk4 == nullptr) {
                fprintf(stderr, "failed to allocate sockIp4\n");
                return -1;
            }
            if(!sk4->setAllInit(ifObj, cfg, interface)) {
                fprintf(stderr, "failed to create transport\n");
                return -1;
            }
            sk = std::move(std::unique_ptr<sockBase>(sk4));
            break;
        }
        case '6': {
            sockIp6 *sk6 = new sockIp6;
            if(sk6 == nullptr) {
                fprintf(stderr, "failed to allocate sockIp6\n");
                return -1;
            }
            if(!sk6->setAllInit(ifObj, cfg, interface)) {
                fprintf(stderr, "failed to create transport\n");
                return -1;
            }
            sk = std::move(std::unique_ptr<sockBase>(sk6));
            break;
        }
        case '2': {
            sockRaw *skr = new sockRaw;
            if(skr == nullptr) {
                fprintf(stderr, "failed to allocate sockRaw\n");
                return -1;
            }
            if(!skr->setAllInit(ifObj, cfg, interface)) {
                fprintf(stderr, "failed to create transport\n");
                return -1;
            }
            sk = std::move(std::unique_ptr<sockBase>(skr));
            break;
        }
    }
    msg.updateParams(prms);
    if(optind == argc) {
        // No arguments left, use batch mode
        // Normal Termination (by kill)
        if(signal(SIGTERM, handle_sig) == SIG_ERR)
            fprintf(stderr, "sig term fails %m\n");
        // Control + C
        if(signal(SIGINT, handle_sig_ctrl) == SIG_ERR)
            fprintf(stderr, "sig init fails %m\n");
        char lineBuf[bufSize];
        while(fgets(lineBuf, bufSize, stdin) != nullptr)
            if(run_line(lineBuf))
                rcv_timeout();
    } else {
        int pkts = 0;
        for(int index = optind; index < argc; index++) {
            if(run_line(argv[index]))
                pkts++;
        }
        for(; pkts > 0; pkts--)
            rcv_timeout();
    }
    sk->close();
    return 0;
}
