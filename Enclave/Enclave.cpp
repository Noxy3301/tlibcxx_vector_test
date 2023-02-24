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

#include "Enclave.h"
#include "Enclave_t.h" /* print_string */
#include <stdarg.h>
#include <stdio.h> /* vsnprintf */
#include <string.h>

#include <stdint.h>
#include <vector>

#include "../Include/consts.h"

/* 
 * printf: 
 *   Invokes OCALL to display the enclave buffer to the terminal.
 */
int printf(const char* fmt, ...)
{
    char buf[BUFSIZ] = { '\0' };
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(buf, BUFSIZ, fmt, ap);
    va_end(ap);
    ocall_print_string(buf);
    return (int)strnlen(buf, BUFSIZ - 1) + 1;
}

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

std::vector<int> e_test;
std::vector<uint64_t> e_index_result;
std::vector<uint64_t> e_iterator_result;

void ecall_vector_init() {
    for (int i = 0; i < TUPLE_NUM; i++) {
        e_test.emplace_back(i);
    }
}

void ecall_vector_loop() {
    for (int i = 0; i < LOOP_NUM; i++) {
        uint64_t ss, ee;
        ss = rdtscp();
        int pos;
        for (pos = 0; pos < TUPLE_NUM; pos++) {
            assert(e_test[pos] == pos);
        }
        ee = rdtscp();
        // printf("index_loop:%d\t%ld\n", i, ee - ss);
        e_index_result.emplace_back(ee - ss);
    }

    for (int i = 0; i < LOOP_NUM; i++) {
        uint64_t ss, ee;
        ss = rdtscp();
        int pos = 0;
        for (auto itr = e_test.begin(); itr != e_test.end(); itr++, pos++) {
            assert((*itr) == pos);
        }
        ee = rdtscp();
        // printf("iterator_loop:%d\t%ld\n", i, ee - ss);
        e_iterator_result.emplace_back(ee - ss);
    }
}

uint64_t ecall_getIndexResult(int index) {
    return e_index_result[index];
}

uint64_t ecall_getIteratorResult(int index) {
    return e_iterator_result[index];
}