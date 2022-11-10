/*
 * Copyright (C) 2011-2021 Intel Corporation. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 *   * Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in
 *     the documentation and/or other materials provided with the
 *     distribution.
 *   * Neither the name of Intel Corporation nor the names of its
 *     contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */


#include <stdio.h>
#include <string.h>
#include <assert.h>

# include <unistd.h>
# include <pwd.h>
# define MAX_PATH FILENAME_MAX

#include "sgx_urts.h"
#include "App.h"
#include "Enclave_u.h"

#include <vector>
#include <thread>
#include <stdint.h>
#include <iostream>
#define TUPLE_NUM 10000000

/* Global EID shared by multiple threads */
sgx_enclave_id_t global_eid = 0;

typedef struct _sgx_errlist_t {
    sgx_status_t err;
    const char *msg;
    const char *sug; /* Suggestion */
} sgx_errlist_t;

/* Error code returned by sgx_create_enclave */
static sgx_errlist_t sgx_errlist[] = {
    {
        SGX_ERROR_UNEXPECTED,
        "Unexpected error occurred.",
        NULL
    },
    {
        SGX_ERROR_INVALID_PARAMETER,
        "Invalid parameter.",
        NULL
    },
    {
        SGX_ERROR_OUT_OF_MEMORY,
        "Out of memory.",
        NULL
    },
    {
        SGX_ERROR_ENCLAVE_LOST,
        "Power transition occurred.",
        "Please refer to the sample \"PowerTransition\" for details."
    },
    {
        SGX_ERROR_INVALID_ENCLAVE,
        "Invalid enclave image.",
        NULL
    },
    {
        SGX_ERROR_INVALID_ENCLAVE_ID,
        "Invalid enclave identification.",
        NULL
    },
    {
        SGX_ERROR_INVALID_SIGNATURE,
        "Invalid enclave signature.",
        NULL
    },
    {
        SGX_ERROR_OUT_OF_EPC,
        "Out of EPC memory.",
        NULL
    },
    {
        SGX_ERROR_NO_DEVICE,
        "Invalid SGX device.",
        "Please make sure SGX module is enabled in the BIOS, and install SGX driver afterwards."
    },
    {
        SGX_ERROR_MEMORY_MAP_CONFLICT,
        "Memory map conflicted.",
        NULL
    },
    {
        SGX_ERROR_INVALID_METADATA,
        "Invalid enclave metadata.",
        NULL
    },
    {
        SGX_ERROR_DEVICE_BUSY,
        "SGX device was busy.",
        NULL
    },
    {
        SGX_ERROR_INVALID_VERSION,
        "Enclave version was invalid.",
        NULL
    },
    {
        SGX_ERROR_INVALID_ATTRIBUTE,
        "Enclave was not authorized.",
        NULL
    },
    {
        SGX_ERROR_ENCLAVE_FILE_ACCESS,
        "Can't open enclave file.",
        NULL
    },
};

/* Check error conditions for loading enclave */
void print_error_message(sgx_status_t ret)
{
    size_t idx = 0;
    size_t ttl = sizeof sgx_errlist/sizeof sgx_errlist[0];

    for (idx = 0; idx < ttl; idx++) {
        if(ret == sgx_errlist[idx].err) {
            if(NULL != sgx_errlist[idx].sug)
                printf("Info: %s\n", sgx_errlist[idx].sug);
            printf("Error: %s\n", sgx_errlist[idx].msg);
            break;
        }
    }
    
    if (idx == ttl)
    	printf("Error code is 0x%X. Please refer to the \"Intel SGX SDK Developer Reference\" for more details.\n", ret);
}

/* Initialize the enclave:
 *   Call sgx_create_enclave to initialize an enclave instance
 */
int initialize_enclave(void)
{
    sgx_status_t ret = SGX_ERROR_UNEXPECTED;
    
    /* Call sgx_create_enclave to initialize an enclave instance */
    /* Debug Support: set 2nd parameter to 1 */
    ret = sgx_create_enclave(ENCLAVE_FILENAME, SGX_DEBUG_FLAG, NULL, NULL, &global_eid, NULL);
    if (ret != SGX_SUCCESS) {
        print_error_message(ret);
        return -1;
    }

    return 0;
}

/* OCall functions */
void ocall_print_string(const char *str)
{
    /* Proxy/Bridge will check the length and null-terminate 
     * the input string to prevent buffer overflow. 
     */
    printf("%s", str);
}


std::vector<int> test;
std::vector<uint64_t> normal_result;
std::vector<uint64_t> iterator_result;

std::vector<uint64_t> e_normal_result;
std::vector<uint64_t> e_iterator_result;

static uint64_t rdtscp() {
    uint64_t rax;
    uint64_t rdx;
    uint32_t aux;
    asm volatile("rdtscp" : "=a"(rax), "=d"(rdx), "=c"(aux)::);
    // store EDX:EAX.
    // 全ての先行命令を待機してからカウンタ値を読み取る．ただし，後続命令は
    // 同読み取り操作を追い越す可能性がある．   
    return (rdx << 32) | rax;
}


/* Application entry */
int SGX_CDECL main(int argc, char *argv[])
{
    (void)(argc);
    (void)(argv);


    /* Initialize the enclave */
    if(initialize_enclave() < 0){
        printf("Enter a character before exit ...\n");
        getchar();
        return -1; 
    }
 

    for (int i = 0; i < TUPLE_NUM; i++) {
        test.emplace_back(i);
    }

    for (int i = 0; i < 10; i++) {
        uint64_t ss, ee;
        ss = rdtscp();
        int pos;
        for (pos = 0; pos < TUPLE_NUM; pos++) {
            assert(test[pos] == pos);
        }
        ee = rdtscp();
        // printf("normal_loop:%d\t%ld\n", i, ee - ss);
        normal_result.emplace_back(ee - ss);
    }

    for (int i = 0; i < 10; i++) {
        uint64_t ss, ee;
        ss = rdtscp();
        int pos = 0;
        for (auto itr = test.begin(); itr != test.end(); itr++, pos++) {
            assert((*itr) == pos);
        }
        ee = rdtscp();
        // printf("iterator_loop:%d\t%ld\n", i, ee - ss);
        iterator_result.emplace_back(ee - ss);
    }

    ecall_vector_init(global_eid);
    ecall_vector_loop(global_eid);

    std::cout << "\t\t" << "enclave\t\t" << "not enclave\t" << "(enclave)/(not enclave)" << std::endl;
    for (int i = 0; i < 10; i++) {
        uint64_t ret;
        ecall_getNormalResult(global_eid, &ret, i);
        e_normal_result.emplace_back(ret);
        std::cout << "normal_loop:" << i << "\t" << ret << "\t" << normal_result[i] << "\t" << (double)ret/(double)normal_result[i] << std::endl;
    }
    uint64_t sum_normal_ne = 0;
    uint64_t sum_normal_e = 0;
    for (int i = 0; i < 10; i++) {
        sum_normal_ne += normal_result[i];
        sum_normal_e += e_normal_result[i];
    }
    std::cout << "[average]\t" << sum_normal_e/10 << "\t" << sum_normal_ne/10 << "\t" <<  (double)sum_normal_e/(double)sum_normal_ne << std::endl;

    std::cout << std::endl;

    for (int i = 0; i < 10; i++) {
        uint64_t ret;
        ecall_getIteratorResult(global_eid, &ret, i);
        e_iterator_result.emplace_back(ret);
        std::cout << "iterator_loop:" << i << "\t" << ret << "\t" << iterator_result[i] << "\t" << (double)ret/(double)iterator_result[i] << std::endl;
    }
    uint64_t sum_iterator_ne = 0;
    uint64_t sum_iterator_e = 0;
    for (int i = 0; i < 10; i++) {
        sum_iterator_ne += iterator_result[i];
        sum_iterator_e += e_iterator_result[i];
    }
    std::cout << "[average]\t" << sum_iterator_e/10 << "\t" << sum_iterator_ne/10 << "\t" <<  (double)sum_iterator_e/(double)sum_iterator_ne << std::endl;

    /* Destroy the enclave */
    sgx_destroy_enclave(global_eid);
    
    // printf("Info: SampleEnclave successfully returned.\n");
    // printf("Enter a character before exit ...\n");
    // getchar();
    return 0;
}

