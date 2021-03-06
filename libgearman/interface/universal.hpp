/*  vim:expandtab:shiftwidth=2:tabstop=2:smarttab:
 * 
 *  Gearmand client and server library.
 *
 *  Copyright (C) 2011 Data Differential, http://datadifferential.com/
 *  Copyright (C) 2008 Brian Aker, Eric Day
 *  All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions are
 *  met:
 *
 *      * Redistributions of source code must retain the above copyright
 *  notice, this list of conditions and the following disclaimer.
 *
 *      * Redistributions in binary form must reproduce the above
 *  copyright notice, this list of conditions and the following disclaimer
 *  in the documentation and/or other materials provided with the
 *  distribution.
 *
 *      * The names of its contributors may not be used to endorse or
 *  promote products derived from this software without specific prior
 *  written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 *  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 *  OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 *  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 *  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 *  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 *  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 *  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 *  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

#pragma once

#include "libgearman/allocator.hpp" 
#include "libgearman/server_options.hpp"
#include "libgearman/interface/packet.hpp"

/**
  @todo this is only used by the server and should be made private.
 */
typedef struct gearman_connection_st gearman_connection_st;
typedef gearman_return_t (gearman_event_watch_fn)(gearman_connection_st *con,
                                                  short events, void *context);

struct gearman_universal_st
{
  struct Options {
    bool dont_track_packets;
    bool non_blocking;

    Options() :
      dont_track_packets(false),
      non_blocking(false)
    { }
  } options;
  gearman_verbose_t verbose;
  uint32_t con_count;
  uint32_t packet_count;
  uint32_t pfds_size;
  uint32_t sending;
  int timeout; // Connection timeout.
  gearman_connection_st *con_list;
  gearman_server_options_st *server_options_list;
  gearman_packet_st *packet_list;
  struct pollfd *pfds;
  gearman_log_fn *log_fn;
  void *log_context;
  gearman_allocator_t allocator;
  struct gearman_vector_st *_namespace;
  struct error_st {
    gearman_return_t rc;
    int last_errno;
    char last_error[GEARMAN_MAX_ERROR_SIZE];

    error_st():
      rc(GEARMAN_SUCCESS),
      last_errno(0)
    {
      last_error[0]= 0;
    }

  } _error;
  int wakeup_fd[2];

  bool is_non_blocking() const
  {
    return options.non_blocking;
  }

  void non_blocking(bool arg_)
  {
    options.non_blocking= arg_;
  }

  const char *error() const
  {
    if (_error.last_error[0] == 0)
    {
      return NULL;
    }

    return static_cast<const char *>(_error.last_error);
  }

  gearman_return_t error_code() const
  {
    return _error.rc;
  }

  void error_code(gearman_return_t rc)
  {
    _error.rc= rc;
  }

  int last_errno() const
  {
    return _error.last_errno;
  }

  void last_errno(int last_errno_)
  {
    _error.last_errno= last_errno_;
  }

  void reset_error()
  {
    _error.rc= GEARMAN_SUCCESS;
    _error.last_errno= 0;
    _error.last_error[0]= 0;
  }

  gearman_return_t option(gearman_universal_options_t option_, bool value)
  {
    switch (option_)
    {
    case GEARMAN_NON_BLOCKING:
      non_blocking(value);
      break;

    case GEARMAN_DONT_TRACK_PACKETS:
      break;

    case GEARMAN_MAX:
    default:
      return GEARMAN_INVALID_COMMAND;
    }

    return GEARMAN_SUCCESS;
  }

  gearman_universal_st(const gearman_universal_options_t *options_= NULL) :
    verbose(GEARMAN_VERBOSE_NEVER),
    con_count(0),
    packet_count(0),
    pfds_size(0),
    sending(0),
    timeout(-1),
    con_list(NULL),
    server_options_list(NULL),
    packet_list(NULL),
    pfds(NULL),
    log_fn(NULL),
    log_context(NULL),
    allocator(gearman_default_allocator()),
    _namespace(NULL)
  {
    wakeup_fd[0]= INVALID_SOCKET;
    wakeup_fd[1]= INVALID_SOCKET;

    if (options_)
    {
      while (*options_ != GEARMAN_MAX)
      {
        /**
          @note Check for bad value, refactor gearman_add_options().
        */
        (void)option(*options_, true);
        options_++;
      }
    }
  }
};

static inline bool gearman_universal_is_non_blocking(gearman_universal_st &self)
{
  return self.is_non_blocking();
}

static inline const char *gearman_universal_error(const gearman_universal_st &self)
{
  return self.error();
}

static inline gearman_return_t gearman_universal_error_code(const gearman_universal_st &self)
{
  return self.error_code();
}

static inline int gearman_universal_errno(const gearman_universal_st &self)
{
  return self.last_errno();
}

static inline void universal_reset_error(gearman_universal_st &self)
{
  self.reset_error();
}
