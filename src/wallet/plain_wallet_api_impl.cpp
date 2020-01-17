// Copyright (c) 2014-2018 Zano Project
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "plain_wallet_api_impl.h"

namespace plain_wallet
{
  typedef epee::json_rpc::response<epee::json_rpc::dummy_result, error> error_response;

  plain_wallet_api_impl::plain_wallet_api_impl(const std::string ip, const std::string port):m_stop(false)
  {
    m_wallet.reset(new tools::wallet2());
    m_wallet->init(ip + ":" + port);
    m_rpc_wrapper.reset(new tools::wallet_rpc_server(*m_wallet));
  }

  plain_wallet_api_impl::~plain_wallet_api_impl()
  {
    if (m_sync_thread.joinable())
      m_sync_thread.join();
  }

  std::string plain_wallet_api_impl::open(const std::string& path, const std::string password)
  {
    error_response err_result = AUTO_VAL_INIT(err_result);
    try
    {
      m_wallet->load(epee::string_encoding::utf8_to_wstring(path), password);
    }
    catch (const tools::error::wallet_load_notice_wallet_restored& e)
    {
      LOG_ERROR("Wallet initialize was with problems, but still worked : " << e.what());
      err_result.error.code = API_RETURN_CODE_FILE_RESTORED;
      return epee::serialization::store_t_to_json(err_result);
    }
    catch (const std::exception& e)
    {
      LOG_ERROR("Wallet initialize failed: " << e.what());
      err_result.error.code = API_RETURN_CODE_FAIL;
      return epee::serialization::store_t_to_json(err_result);
    }
    epee::json_rpc::response<open_wallet_response, epee::json_rpc::dummy_error> ok_response = AUTO_VAL_INIT(ok_response);
    m_wallet->get_recent_transfers_history(ok_response.result.recent_history.history, 0, 20, ok_response.result.recent_history.total_history_items);
    m_wallet->get_unconfirmed_transfers(ok_response.result.recent_history.history);
    tools::get_wallet_info(*m_wallet, ok_response.result.wi);
    return epee::serialization::store_t_to_json(ok_response);
  }

  std::string plain_wallet_api_impl::restore(const std::string& seed, const std::string& path, const std::string password)
  {
    error_response err_result = AUTO_VAL_INIT(err_result);
    try
    {
      m_wallet->restore(epee::string_encoding::utf8_to_wstring(path), password, seed);
    }
    catch (const tools::error::wallet_load_notice_wallet_restored& e)
    {
      LOG_ERROR("Wallet initialize was with problems, but still worked : " << e.what());
      err_result.error.code = API_RETURN_CODE_FILE_RESTORED;
      return epee::serialization::store_t_to_json(err_result);
    }
    catch (const std::exception& e)
    {
      LOG_ERROR("Wallet initialize failed: " << e.what());
      err_result.error.code = API_RETURN_CODE_FAIL;
      return epee::serialization::store_t_to_json(err_result);
    }
    epee::json_rpc::response<open_wallet_response, epee::json_rpc::dummy_error> ok_response = AUTO_VAL_INIT(ok_response);
    tools::get_wallet_info(*m_wallet, ok_response.result.wi);
    return epee::serialization::store_t_to_json(ok_response);
  }

  std::string plain_wallet_api_impl::generate(const std::string& path, const std::string password)
  {
    error_response err_result = AUTO_VAL_INIT(err_result);
    try
    {
      m_wallet->generate(epee::string_encoding::utf8_to_wstring(path), password);
    }
    catch (const tools::error::wallet_load_notice_wallet_restored& e)
    {
      LOG_ERROR("Wallet initialize was with problems, but still worked : " << e.what());
      err_result.error.code = API_RETURN_CODE_FILE_RESTORED;
      return epee::serialization::store_t_to_json(err_result);
    }
    catch (const std::exception& e)
    {
      LOG_ERROR("Wallet initialize failed: " << e.what());
      err_result.error.code = API_RETURN_CODE_FAIL;
      return epee::serialization::store_t_to_json(err_result);
    }
    epee::json_rpc::response<open_wallet_response, epee::json_rpc::dummy_error> ok_response = AUTO_VAL_INIT(ok_response);
    tools::get_wallet_info(*m_wallet, ok_response.result.wi);
    return epee::serialization::store_t_to_json(ok_response);
  }

  bool plain_wallet_api_impl::start_sync_thread()
  {
    m_sync_thread = std::thread([&]() 
    {
      m_wallet->refresh(m_stop);
    });
  }

  bool plain_wallet_api_impl::cancel_sync_thread()
  {
    m_stop = true;
  }

  std::string plain_wallet_api_impl::get_sync_status()
  {
    m_wallet->get_sync_progress();
  }

  std::string plain_wallet_api_impl::sync()
  {
    m_wallet->refresh(m_stop);
  }
  std::string plain_wallet_api_impl::invoke(const std::string& params)
  {
    m_rpc_wrapper->handle_http_request_map()
  }


}