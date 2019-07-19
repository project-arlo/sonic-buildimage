/*
* iccp_cmd.c
*
* Copyright(c) 2016-2019 Nephos/Estinet.
*
* This program is free software; you can redistribute it and/or modify it
* under the terms and conditions of the GNU General Public License,
* version 2, as published by the Free Software Foundation.
*
* This program is distributed in the hope it will be useful, but WITHOUT
* ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
* FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
* more details.
*
* You should have received a copy of the GNU General Public License along with
* this program; if not, see <http://www.gnu.org/licenses/>.
*
* The full GNU General Public License is included in this distribution in
* the file called "COPYING".
*
*  Maintainer: jianjun, grace Li from nephos
*/

#include <net/if.h>
#include <sys/queue.h>
#include <ctype.h>

#include "../include/iccp_csm.h"
#include "../include/msg_format.h"
#include "../include/system.h"

#include "../include/iccp_cmd_show.h"
#include "../include/iccp_cli.h"
#include "../include/logger.h"

int set_mc_lag_by_id(uint16_t mid)
{
    int ret = 0;
    struct CSM* csm = NULL;

    csm = system_get_csm_by_mlacp_id(mid);
    if (!csm)
    {
        csm = (struct CSM*)iccp_get_csm();
        if (csm == NULL)
        {
            return -1;
        }

        ret = set_mc_lag_id(csm, mid);

        return ret;
    }

    return ret;
}

#define CONFIG_LINE_LEN  512

int iccp_config_from_command(char * line)
{
    char *cp, *start;
    char token[64];
    int slen;
    static int mid = 0;
    char *end;

    if (line == NULL)
        return 0;

    cp = line;

    /* Skip white spaces. */
    while (isspace ((int) *cp) && *cp != '\0')
        cp++;

    /* Return if there is only white spaces */
    if (*cp == '\0')
        return 0;

    end = cp;

     /* Skip end white spaces. */
    while (!isspace ((int) *end) && *end != '\0')
        end++;

    *end = '\0';
    /*mc-lag id*/
    if (strncmp(cp,"mclag_id:",9) == 0 )
    {
        cp += 9;
        mid = atoi(cp);
        set_mc_lag_by_id(mid);
    }
    else if (strncmp(cp,"local_ip:",9) == 0) /*local ip*/
    {
        cp += 9;
        set_local_address(mid,cp);
    }
    else if (strncmp(cp,"peer_ip:",8) == 0) /*peer ip*/
    {
        cp += 8;
        set_peer_address(mid,cp);
    }
    else if(strncmp(cp,"peer_link:",10) == 0)
    {
        cp += 10;
        set_peer_link(mid,cp);
    }
    else if(strncmp(cp,"mclag_interface:",16) == 0)
    {
        cp += 16;
       
        while (1) 
        {
            start = cp;
            while (!(*cp == ',' || *cp == '\r' || *cp == '\n') &&
             *cp != '\0')
                cp++;
            slen = cp - start;
            strncpy (token, start, slen);
            *(token + slen) = '\0';
            iccp_cli_attach_mclag_domain_to_port_channel(mid, token);

            while ((isspace ((int) *cp) || *cp == '\n' || *cp == '\r' || *cp == ',') &&
             *cp != '\0')
                cp++;

            if (*cp == '\0')
                break;
        }
    }
    else
    {
        /*error*/
    }

    return 1;
}

/* Configration make from file. */
int
iccp_config_from_file (char *config_default_dir)
{
    FILE *confp = NULL;
    char command_buf[CONFIG_LINE_LEN];

    confp = fopen (config_default_dir, "r");
    if (confp == NULL)
        return (1);

    while (fgets (command_buf, CONFIG_LINE_LEN, confp))
    {
        iccp_config_from_command(command_buf);
    }
    fclose(confp);

    return 0;
}

