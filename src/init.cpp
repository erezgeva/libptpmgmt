/* SPDX-License-Identifier: LGPL-3.0-or-later
   SPDX-FileCopyrightText: Copyright © 2021 Erez Geva <ErezGeva2@gmail.com> */

/** @file
 * @brief Init a pmc application
 *
 * @author Erez Geva <ErezGeva2@@gmail.com>
 * @copyright © 2021 Erez Geva
 *
 */

#include "comp.h"
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#include "init.h"

__PTPMGMT_NAMESPACE_BEGIN

void Init::close()
{
    SockBase *s = sk();
    if(s != nullptr)
        s->close();
}

int Init::process(const Options &opt)
{
    char net_select = opt.get_net_transport();
    // handle configuration file
    if(opt.have('f') && !m_cfg.read_cfg(opt.val('f')))
        return -1;
    if(net_select == 0)
        net_select = m_cfg.network_transport();
    string interface;
    if(opt.have('i') && !opt.val('i').empty())
        interface = opt.val('i');
    IfInfo ifObj;
    MsgParams prms = m_msg.getParams();
    if(net_select != 'u') {
        if(interface.empty()) {
            PTPMGMT_ERROR("missing interface");
            return -1;
        }
        if(!ifObj.initUsingName(interface))
            return -1;
        Binary clockIdentity(ifObj.mac());
        clockIdentity.eui48ToEui64();
        clockIdentity.copy(prms.self_id.clockIdentity.v);
        prms.self_id.portNumber = 1;
        m_use_uds = false;
    }
    if(opt.have('b'))
        prms.boundaryHops = opt.val_i('b');
    else
        prms.boundaryHops = 1;
    if(opt.have('d'))
        prms.domainNumber = opt.val_i('d');
    else
        prms.domainNumber = m_cfg.domainNumber();
    if(opt.have('t'))
        prms.transportSpecific = strtol(opt.val_c('t'), nullptr, 16);
    else
        prms.transportSpecific = m_cfg.transportSpecific();
    prms.useZeroGet = opt.val('z') == "1";
    m_net_select = net_select;
    switch(net_select) {
        case 'u': {
            SockUnix *sku = new SockUnix;
            if(sku == nullptr) {
                PTPMGMT_ERROR("failed to allocate SockUnix");
                return -1;
            }
            m_sk.reset(sku);
            string uds_address;
            if(opt.have('s'))
                uds_address = opt.val('s');
            else
                uds_address = m_cfg.uds_address();
            if(!sku->setDefSelfAddress() || !sku->init() ||
                !sku->setPeerAddress(uds_address))
                return -1;
            pid_t pid = getpid();
            prms.self_id.clockIdentity.v[6] = (pid >> 24) & 0xff;
            prms.self_id.clockIdentity.v[7] = (pid >> 16) & 0xff;
            prms.self_id.portNumber = pid & 0xffff;
            m_use_uds = true;
            break;
        }
        default:
            m_net_select = '4';
            FALLTHROUGH;
        case '4': {
            SockIp4 *sk4 = new SockIp4;
            if(sk4 == nullptr) {
                PTPMGMT_ERROR("failed to allocate SockIp4");
                return -1;
            }
            m_sk.reset(sk4);
            if(!sk4->setAll(ifObj, m_cfg))
                return -1;
            if(opt.have('T') && !sk4->setUdpTtl(opt.val_i('T')))
                return -1;
            if(!sk4->init())
                return -1;
            break;
        }
        case '6': {
            SockIp6 *sk6 = new SockIp6;
            if(sk6 == nullptr) {
                PTPMGMT_ERROR("failed to allocate SockIp6");
                return -1;
            }
            m_sk.reset(sk6);
            if(!sk6->setAll(ifObj, m_cfg))
                return -1;
            if(opt.have('T') && !sk6->setUdpTtl(opt.val_i('T')))
                return -1;
            if(opt.have('S') && !sk6->setScope(opt.val_i('S')))
                return -1;
            if(!sk6->init())
                return -1;
            break;
        }
        case '2': {
            SockRaw *skr = new SockRaw;
            if(skr == nullptr) {
                PTPMGMT_ERROR("failed to allocate SockRaw");
                return -1;
            }
            m_sk.reset(skr);
            if(!skr->setAll(ifObj, m_cfg))
                return -1;
            if(opt.have('P') &&
                !skr->setSocketPriority(opt.val_i('P')))
                return -1;
            if(opt.have('M')) {
                Binary mac;
                if(!mac.fromMac(opt.val('M'))) {
                    PTPMGMT_ERROR("Wrong MAC address '%s'", opt.val('M').c_str());
                    return -1;
                }
                if(!skr->setPtpDstMac(mac))
                    return -1;
            }
            if(!skr->init())
                return -1;
            break;
        }
    }
    m_msg.updateParams(prms);
    PTPMGMT_ERROR_CLR;
    return 0;
}

__PTPMGMT_NAMESPACE_END

__PTPMGMT_NAMESPACE_USE;

extern "C" {

#include "c/init.h"

    extern ptpmgmt_cfg ptpmgmt_cfg_alloc_wrap(const ConfigFile &cfg);
    extern ptpmgmt_msg ptpmgmt_msg_alloc_wrap(const Message &msg);
    extern ptpmgmt_sk ptpmgmt_sk_alloc_wrap(ptpmgmt_socket_class type,
        SockBase *sko);

    static void ptpmgmt_init_free(ptpmgmt_init me)
    {
        if(me != nullptr) {
            if(me->sCfg != nullptr) {
                me->sCfg->free(me->sCfg);
                free(me->sCfg);
                me->sCfg = nullptr;
            }
            if(me->sMsg != nullptr) {
                me->sMsg->free(me->sMsg);
                free(me->sMsg);
                me->sMsg = nullptr;
            }
            if(me->sSk != nullptr) {
                me->sSk->free(me->sSk);
                free(me->sSk);
                me->sSk = nullptr;
            }
            if(me->_this != nullptr) {
                delete(Init *)me->_this;
                me->_this = nullptr;
            }
            free(me);
        }
    }
    static void ptpmgmt_init_close(ptpmgmt_init me)
    {
        if(me != nullptr && me->_this != nullptr)
            ((Init *)me->_this)->close();
    }
    static int ptpmgmt_init_process(ptpmgmt_init me, const_ptpmgmt_opt opt)
    {
        if(me != nullptr && me->_this != nullptr && opt != nullptr)
            return ((Init *)me->_this)->process(*(const Options *)opt->_this);
        return -1;
    }
    static ptpmgmt_cfg ptpmgmt_init_cfg(ptpmgmt_init me)
    {
        if(me != nullptr && me->_this != nullptr) {
            if(me->sCfg == nullptr)
                me->sCfg = ptpmgmt_cfg_alloc_wrap(((Init *)me->_this)->cfg());
            return me->sCfg;
        }
        return nullptr;
    }
    static ptpmgmt_msg ptpmgmt_init_msg(ptpmgmt_init me)
    {
        if(me != nullptr && me->_this != nullptr) {
            if(me->sMsg == nullptr)
                me->sMsg = ptpmgmt_msg_alloc_wrap(((Init *)me->_this)->msg());
            return me->sMsg;
        }
        return nullptr;
    }
    static ptpmgmt_sk ptpmgmt_init_sk(ptpmgmt_init me)
    {
        if(me != nullptr && me->_this != nullptr) {
            Init *pi = (Init *)me->_this;
            if(me->sSk == nullptr) {
                ptpmgmt_socket_class type;
                switch(pi->getNetSelect()) {
                    case 'u':
                        type = ptpmgmt_SockUnix;
                        break;
                    case '4':
                        type = ptpmgmt_SockIp4;
                        break;
                    case '6':
                        type = ptpmgmt_SockIp6;
                        break;
                    case '2':
                        type = ptpmgmt_SockRaw;
                        break;
                    default:
                        return nullptr;
                }
                SockBase *s = pi->sk();
                me->sSk = ptpmgmt_sk_alloc_wrap(type, s);
            }
            return me->sSk;
        }
        return nullptr;
    }
    static char ptpmgmt_init_getNetSelect(const_ptpmgmt_init me)
    {
        if(me != nullptr && me->_this != nullptr)
            return ((Init *)me->_this)->getNetSelect();
        return 0;
    }
    static bool ptpmgmt_init_use_uds(ptpmgmt_init me)
    {
        if(me != nullptr && me->_this != nullptr)
            return ((Init *)me->_this)->use_uds();
        return false;
    }
    ptpmgmt_init ptpmgmt_init_alloc()
    {
        ptpmgmt_init me = (ptpmgmt_init)malloc(sizeof(ptpmgmt_init_t));
        if(me == nullptr)
            return nullptr;
        me->_this = (void *)(new Init);
        if(me->_this == nullptr) {
            free(me);
            return nullptr;
        }
#define C_ASGN(n) me->n = ptpmgmt_init_##n
        C_ASGN(free);
        C_ASGN(close);
        C_ASGN(process);
        C_ASGN(cfg);
        C_ASGN(msg);
        C_ASGN(sk);
        C_ASGN(getNetSelect);
        C_ASGN(use_uds);
        me->sCfg = nullptr;
        me->sMsg = nullptr;
        me->sSk = nullptr;
        return me;
    }
}
