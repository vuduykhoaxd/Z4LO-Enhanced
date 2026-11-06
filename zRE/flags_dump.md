# Zalo PC 26.10.20 — Hidden Flags & Features Dump

Generated: 2026-11-05T18:38:07.926Z

## Stats

- **liveConfigKeys**: 607
- **settingsFeatureKeys**: 336
- **settingsFeatureKeysMappedToLiveConfig**: 275
- **enableIdentifiersInConfigModule**: 279
- **enableIdentifiersInWholeAsar**: 463
- **liveFeatureFlags**: 162
- **localStorageKeys**: 313
- **tabBannerCacheKeys**: 221

## How Zalo's hidden flags work

1. On login, the server sends a `server-info` payload. Its settings block is named `setttings` (**three** t's — a typo in Zalo's code) and `setttings.features` is the server feature-flag object.
2. Zalo merges those values into one app-wide config object. At runtime that object is the `default` export of webpack module **`NDmK`** — 607 keys.
3. `window.$zFeatures` is **not** the flag list. It only exposes 15 native bridge features (zaloCall, libvips, ocr, ...).
4. `window.__tabBannerRemoteConfig__` / `remoteConfigsCache._cache` is a separate remote-config system (221 keys: banners, retry, downloads, ...). `enable_guggy` is **not** there — it comes from `setttings.features`.
5. Because the config object is a normal mutable JS object, any flag can be read or changed at runtime. Zalo re-applies server values after a reload, so runtime overrides last until reload/restart.

## Toggle any flag at runtime (tested)

The page uses webpack 4 (`window.webpackJsonp`). This snippet installs a reference to the internal `__webpack_require__`, finds the config module by its marker, and changes a flag:

```js
(function () {
  if (!window.__zreq) {
    var chunkId = [Math.floor(Math.random() * 1e6) + 900000];
    var moduleId = '__z_inspect_' + Math.random().toString(36).slice(2);
    var modules = {};
    modules[moduleId] = function (module, exports, __webpack_require__) {
      window.__zreq = __webpack_require__;
    };
    window.webpackJsonp.push([chunkId, modules, [[moduleId]]]);
  }

  var cfgId = null;
  for (var id of Object.keys(window.__zreq.m)) {
    var src = '';
    try { src = window.__zreq.m[id].toString(); } catch (e) { continue; }
    if (src.indexOf('enable_guggy:0') !== -1) { cfgId = id; break; }
  }

  var cfg = window.__zreq(cfgId).default;
  cfg.enable_guggy = 1;          // change any flag here
  return cfg.enable_guggy;
})()
```

Verified on Zalo 26.10.20: config module id `NDmK`, `enable_guggy` changed from `0` to `1`.

## Notable hidden flags (current live values)

| Flag | Current | What it controls |
|---|---|---|
| `enable_guggy` | `0` | Guggy sticker search / @guggy command |
| `enable_sticker_hi` | `true` | High-quality stickers |
| `enable_hi_group_member` | `false` | High member count groups |
| `enable_archived_chat` | `1` | Archived chats |
| `enable_hidden_chat` | `true` | Hidden chats |
| `enable_busy_mode` | `0` | Busy mode |
| `enable_multi_window` | `1` | Multiple app windows |
| `enable_multi_del_msg` | `1` | Delete multiple messages |
| `enable_multi_recall` | `true` | Recall multiple messages |
| `enable_undo` | `1` | Undo message |
| `enable_photo_editor` | `0` | Photo editor |
| `enable_photoviewer_popup` | `0` | Photo viewer popup |
| `enable_global_search_phase_2` | `0` | Global search phase 2 |
| `enable_new_privacy_stranger_chat` | `1` | New stranger chat privacy |
| `enable_new_ui_stranger` | `1` | New stranger UI |
| `enable_report_user` | `1` | Report user |
| `enable_pc_usage` | `0` | PC usage tracking |
| `enable_pc_info` | `0` | PC info collection |
| `enable_get_all_process` | `0` | Collect all processes |
| `enable_mac_call` | `0` | Mac call |
| `enable_group_call_for_user` | `1` | Group call |
| `enable_group_call_entry_for_group` | `100` | Group call entry point |
| `enable_file_preview` | `1` | File preview |
| `enable_file_preview_google` | `1` | Google file preview |
| `enable_file_preview_microsoft` | `1` | Microsoft file preview |
| `enable_predownload_file` | `true` | Pre-download files |
| `enable_upload_file_w_thumb` | `1` | Upload file with thumbnail |
| `enable_auto_hide_scrollbar_conv_list` | `false` | Auto-hide conversation scrollbar |
| `enable_reminder_1_1` | `1` | 1-1 reminders |
| `enable_board_suggestion` | `true` | Board suggestion |
| `enable_blacklist_mention` | `true` | Mention blacklist |
| `enable_emoji_link` | `0` | Emoji links |
| `enable_image_link_preview` | `false` | Image link preview |
| `enable_typing_icon` | `false` | Typing indicator |
| `enable_unread_v2` | `true` | Unread counter v2 |
| `enable_archived_chat` | `1` | Archived conversations |
| `enable_sync_pinned` | `true` | Sync pinned conversations |
| `enable_close_mute_banner` | `0` | Mute banner |
| `enable_idle_reload` | `1` | Idle reload |
| `enable_abort_poll_timeout` | `true` | Abort polling timeout |

## AI Sticker (`ai_sticker`) — verified at runtime

- Server sends `setttings.features.ai_sticker = {"enable":0}`. It is stored by webpack module **`kuNG`**, not in the big `NDmK` config object.
- `enable` is a **bitmask**: `1` = render AI-sticker messages, `2` = AI tab in the sticker picker, `3` = both.
- The validator only allows `oneOf([0,1])`, and `validateWithFallback(..., default, true)` falls back to the bundle default `{enable:1}`. So the `2` bit is **unreachable through config** — the tab is effectively hard-disabled.
- Mutating the live config object directly bypasses validation: `window.__zreq("kuNG").a().config.enable = 3`. After that the AI section really renders inside the sticker picker (class `.ai-sticker-promotion`).
- What it shows: `"Chế tạo" sticker theo cách của bạn.` / `Mở Zalo trên điện thoại để thử ngay.` — a promotion card only. `mini_app_urL` is empty and the AI sticker list is empty.

**Conclusion:** on PC 26.10.20 the flag only enables a promo card in the sticker picker. Actual AI sticker generation is mobile / server side, so it is **not usable on PC** with the client flag alone.

## All boolean-like config flags (live values)

| Flag | Value |
|---|---|
| `allow_submit_identity` | `true` |
| `allow_two_visible` | `false` |
| `always_renew_video` | `false` |
| `banner_notification` | `1` |
| `context_menu_mode` | `1` |
| `conv_ux_force_new` | `1` |
| `conv_ux_more_menu` | `0` |
| `conv_ux_switcher` | `0` |
| `default_banner_on_response` | `true` |
| `disable_clone_video` | `false` |
| `disable_nav_keys_at_ssl_on_platform` | `0` |
| `disable_unmark_ff_on_socket_ovf` | `true` |
| `draw_unread_icon` | `false` |
| `enable_abort_poll_timeout` | `true` |
| `enable_add_identity` | `true` |
| `enable_add_staredDttm_logic` | `true` |
| `enable_api_verify_fr_list` | `true` |
| `enable_archived_chat` | `1` |
| `enable_async_preparse` | `false` |
| `enable_auto_hide_scrollbar_conv_list` | `false` |
| `enable_avatar_log` | `0` |
| `enable_avatar_v2` | `0` |
| `enable_blacklist_mention` | `true` |
| `enable_board_suggestion` | `true` |
| `enable_busy_mode` | `0` |
| `enable_cache_fss` | `1` |
| `enable_cache_parse_link_fail` | `0` |
| `enable_check_queue_ctrl` | `true` |
| `enable_check_storage` | `true` |
| `enable_close_mute_banner` | `0` |
| `enable_conversation_settings` | `1` |
| `enable_DFE` | `true` |
| `enable_DFE_admin` | `true` |
| `enable_DFE_sender` | `true` |
| `enable_discovery` | `0` |
| `ENABLE_DISCOVERY_SUGGEST` | `0` |
| `enable_emoji_link` | `0` |
| `enable_encrypt_param` | `true` |
| `enable_feature_alias` | `1` |
| `enable_feature_alias_v2` | `true` |
| `enable_file_photo_send_msg_photo` | `1` |
| `enable_file_preview` | `1` |
| `enable_file_preview_google` | `1` |
| `enable_file_preview_microsoft` | `1` |
| `enable_file_store_comp` | `1` |
| `enable_forward_module` | `1` |
| `enable_get_all_process` | `0` |
| `enable_get_last_seen` | `1` |
| `enable_get_last_seen_v2` | `true` |
| `enable_get_size_video` | `0` |
| `enable_global_search_phase_2` | `0` |
| `enable_group_call_entry_for_group` | `100` |
| `enable_group_call_for_user` | `1` |
| `enable_group_photo_skeleton` | `1` |
| `enable_group_profile` | `1` |
| `enable_guggy` | `0` |
| `enable_hi_group_member` | `false` |
| `enable_hidden_chat` | `true` |
| `enable_idle_reload` | `1` |
| `enable_image_link_preview` | `false` |
| `enable_interval_log` | `1` |
| `enable_ipc_call` | `true` |
| `enable_keep_msg_info_DFE` | `true` |
| `enable_log_search` | `false` |
| `enable_mac_call` | `0` |
| `enable_merge_msg_offline_status` | `true` |
| `enable_mini_info` | `true` |
| `enable_multi_del_msg` | `1` |
| `enable_multi_recall` | `true` |
| `enable_multi_window` | `1` |
| `enable_new_capture` | `true` |
| `enable_new_fss` | `1` |
| `enable_new_privacy_stranger_chat` | `1` |
| `enable_new_rule_sort_recent_search` | `0` |
| `enable_new_ui_stranger` | `1` |
| `enable_nretry` | `true` |
| `enable_optimize_search` | `1` |
| `enable_parse_msg_v2` | `1` |
| `enable_parse_patterns` | `1` |
| `enable_pc_info` | `0` |
| `enable_pc_usage` | `0` |
| `enable_photo_editor` | `0` |
| `enable_photoviewer_popup` | `0` |
| `enable_photoviewer_reaction` | `1` |
| `enable_photoviewer_timeline_slider_wheel` | `1` |
| `enable_popup_confim_del` | `false` |
| `enable_popup_copy_excel` | `true` |
| `enable_predownload_file` | `true` |
| `enable_preview_device_incall` | `1` |
| `enable_preview_device_outgoing` | `1` |
| `enable_privacy_settings` | `true` |
| `enable_profile_loading` | `1` |
| `enable_reminder_1_1` | `1` |
| `enable_report_user` | `1` |
| `enable_retry_no_network` | `true` |
| `enable_retry_no_network_confirm` | `1` |
| `enable_retry_timeout` | `true` |
| `enable_search_stranger` | `1` |
| `enable_search_top_result` | `0` |
| `enable_sticker_hi` | `true` |
| `enable_stranger_alias` | `1` |
| `enable_suggest_mention` | `true` |
| `enable_swap_domain` | `true` |
| `enable_syc_settings` | `1` |
| `enable_sync_cloud_first_login` | `true` |
| `enable_sync_del_conv` | `1` |
| `enable_sync_del_msg` | `1` |
| `enable_sync_pinned` | `true` |
| `enable_todo` | `1` |
| `enable_typing_icon` | `false` |
| `enable_undo` | `1` |
| `enable_unread_v2` | `true` |
| `enable_unstable_connecting` | `0` |
| `enable_upload_file_w_thumb` | `1` |
| `enable_warning_stranger` | `1` |
| `enableAutoSubmitLog` | `false` |
| `enableCall` | `true` |
| `enableFeatures` | `{"showSeenDeliveredsStatus":true,"showSeenDeliveredsPreview":true,"prepareDataSeenDelivereds":true,"showWarningTimeInvalid":true,"checkSwitchDomain":true,"logTodoRelated":true}` |
| `enableFetchHistoryMessage` | `false` |
| `enableFileGlobalSearch` | `true` |
| `enableFriendSuggestion` | `false` |
| `enableHistoryMessage` | `true` |
| `enableImageRetry` | `true` |
| `enableInvitePc` | `1` |
| `enableMemoryMonitor` | `false` |
| `enableMultiSelect` | `true` |
| `enablePollingLastAction` | `true` |
| `enablePollingLastActionCtrl` | `true` |
| `enableTag` | `true` |
| `enableVideoCall` | `true` |
| `file_upload_show_icon_1GB` | `true` |
| `flag_new_search` | `true` |
| `get_last_seen_onlogin` | `false` |
| `isEnableTieqViet` | `false` |
| `isOffBlockFeature` | `true` |
| `isOffCloseFriendFeature` | `true` |
| `isOffLanFriends` | `true` |
| `isOffSearchMedia` | `false` |
| `isOffSendToMe` | `false` |
| `isOffTetEffect` | `true` |
| `isOffVoiceRecord` | `true` |
| `myDocumentNewIcon` | `1` |
| `new_layout_group_photo_2x2` | `true` |
| `NEW_PROFILE_V3` | `1` |
| `new_ui_ecard` | `1` |
| `off_message_reaction` | `false` |
| `off_silent_fetch_group` | `false` |
| `offGame` | `true` |
| `quote_icon` | `0` |
| `render_content_with_zalo_emoji` | `true` |
| `retry_err_unknown_exception` | `true` |
| `search_multi_phonenumber` | `1` |
| `should_force_genlist_conv` | `true` |
| `show_confirm_del` | `1` |
| `show_sticker_suggest_hint` | `0` |
| `showNewWelcomeScreen` | `1` |
| `showOnboarding` | `0` |
| `showRender` | `false` |
| `suggest_enable_suggest_file` | `false` |
| `suggest_enable_suggest_link` | `false` |
| `suggest_enable_suggest_photo` | `false` |
| `use_net_error` | `0` |

## Server feature keys (`setttings.features`)

These are the keys Zalo accepts from the server. `enable_guggy` belongs to this list. The "live key" column is the matching key inside the runtime config object.

| Server key | Live config key | Value |
|---|---|---|
| `ack_evict_queues` | `ack_evict_queues` | `["8"]` |
| `ack_seen` | `ack_seen` | `{"v2_enable":true,"v1_retry":false,"from_me":true,"queue_enable":true,"queue_delay":150}` |
| `add_params_fw_send2me` | `add_params_fw_send2me` | `1` |
| `ai_sticker` | — | — |
| `alias` | `alias` | `-1` |
| `alias_v2` | `alias_v2` | `{"periodic_sync_time":604800,"periodic_sync_max_retry":3}` |
| `all_process_interval` | `all_process_interval` | `3960000` |
| `allow_two_visible` | `allow_two_visible` | `false` |
| `async_forward` | `async_forward` | `{"on":1,"open_mode":1,"timeout_wait_ctrl":120000,"checked_url_cache_expiration_time":3600000,"checking_url_timeout":10000,"disable_cache_check_exist":1,"maximum_requests_in_a_while":5,"retry_by_upload":1,"enable_monitor_forward_file_noise":1,"enable_ignore_localpath_file_noise":1,"enable_de_noise_in_forward":1,"use_cache_for_upload":0,"enable_verify_checksum_forward_cross":1}` |
| `auto_download_media` | `auto_download_media` | `{"voice":1,"video":1}` |
| `auto_download_media_transfered` | `auto_download_media_transfered` | `{"enable":1,"validDays":13}` |
| `auto_relaunch_mode` | — | — |
| `auto_relaunch_timeout` | — | — |
| `avatar_link_renew` | `avatarLinkRenew` | `{"enabled":true,"enabled_renew_by_params":false,"enabled_renew_my_cloud_profile":false,"renew_days_before_expire":3,"storage_days_before_expire":7,"batch_size":100,"max_retries":5,"debounce_time_ms":1500,"is_persist":false,"num_need_to_prune":50,"num_expire_soon_need_to_renew":20}` |
| `avoid_flicker_p2` | — | — |
| `b2s_config` | `b2s_config` | `{"enable":0,"type":0}` |
| `bank_parser` | `bank_parser` | `{"enable":1,"enable_e2ee":0}` |
| `banner_notification` | `banner_notification` | `1` |
| `block_friend` | `block_friend` | `"1765712685165040"` |
| `block_msg_call_setting` | `block_msg_call_setting` | `{"enable_block_msg_call_setting":1}` |
| `broadcast` | `broadcast` | `{"maxTimesAWeek":4500,"maxTimesADay":30,"broadcastTag":["khachhang"],"maxTimesAMonth":9000,"broadcastAllLabels":true,"delayTime":10,"maxFriend":30,"isEnable":true}` |
| `broadcast_resend_config` | `broadcast_resend_config` | `{"max_package":10,"enable":0}` |
| `busy_mode_idle_time` | `busy_mode_idle_time` | `15` |
| `catId` | — | — |
| `certs` | — | — |
| `character_ava` | `character_ava` | `1` |
| `chat_aggressive_send_seen` | `chat_aggressive_send_seen` | `true` |
| `chat_media_auto_download` | `chat_media_auto_download` | `{"enable_new_domain":1,"t-md":"I-0-21HqAjVguGx1RpUr7QgXZ00jLlUmghoC9h3YzRhsTCncCo2tp7MsZjGOm815LhaS1dO75G","prefix":"pca-","allow_domain":"zdn.vn"}` |
| `chat_show_prepare_seen_delivereds` | — | — |
| `chat_show_preview_seen_delivereds` | — | — |
| `chat_show_seen_delivereds` | — | — |
| `check_switch_domain` | — | — |
| `cloud_log_config` | — | — |
| `cloud_send2me` | `cloud_send2me` | `{"enable":1,"limitSize":104857600,"duration_expired":7,"showDebugIndicator":false}` |
| `cloud_setting` | — | — |
| `cloud_upload_config` | `cloud_upload_config` | `{"zrf_upload_size":1073741824,"enable_zrf_upload":1,"milestone":1209600000,"upload_pause":600000,"enable":1,"db_page_load":10,"delay_process_queue_item":1000,"lifetime":600000,"interval_check_db":600000}` |
| `common_gr` | `common_gr` | `{"profile":1,"fr_list":1}` |
| `common_settings` | — | — |
| `config_fetch_fr_list` | — | — |
| `connect_src_rgx` | `connect_src_rgx` | `"(.*.zalo.me)|(.*.zdn.vn)|(.*.zadn.vn)|(.*.zaloapp.com)|(.*.dlfl.me)|(.*.dlmd.me)|(.*.mdchat.me)|(.*.dlfl.vn)|(.*.dlfl.info)|(.*.flchat.vn)|(.*.pchat.vn)"` |
| `contactTabV2` | `contactTabV2` | `{"enable_rule_last_view_friend_req":0,"enable_contact_tab_v2":1,"seen_friendrequest":1,"seen_friendrequest_chat_view":0}` |
| `context_menu_mode` | `context_menu_mode` | `1` |
| `conv_label` | `conv_label` | `"-1"` |
| `conv_nav` | `conv_nav` | `{"enable_jump_last_pos":0,"enable_jump_unread":1,"enable_jump_new":1,"jump_last_pos_keep_for":360000,"jump_last_pos_hide_after":9000,"jump_unread_hide_after":12000,"show_new_msg_line_pos":0}` |
| `conv_unread_count` | — | — |
| `conv_ux_force_new` | `conv_ux_force_new` | `1` |
| `conv_ux_more_menu` | `conv_ux_more_menu` | `0` |
| `conv_ux_switcher` | `conv_ux_switcher` | `0` |
| `conv_ux_ver` | `conv_ux_ver` | `3` |
| `cross_setting` | `cross_setting` | `{"offFeature":false,"isMobileSupport":true,"cfTimeout":180,"waitDbTimeout":1200,"closeSuccessTimeout":2,"autoRetryApi":2,"timeoutRetry":60000,"timeoutKey":1,"enableOverQueue":true,"enableManual":true,"enableResume":true,"enablePCOld":false,"enableFirstTimeUse":true,"delayNoti":[3,7,14,30,60,90],"retrySendCf":10,"useDevDomain":0,"show_msg_src":false,"enable_break_folder_db_as_session":true,"useTheme":"v2","enable":true}` |
| `db_queue` | — | — |
| `debounce_search_msg` | `debounce_search_msg` | `500` |
| `debounce_search_msg_optimize` | `debounce_search_msg_optimize` | `2000` |
| `disable_clone` | — | — |
| `disable_nav_keys_at_ssl_on_platform` | `disable_nav_keys_at_ssl_on_platform` | `0` |
| `download_concur` | `downloadConcur` | `{"profiles":[{"src":"auto","run":1,"enable":1},{"src":"dev","run":3,"enable":1},{"src":"urgent","run":10,"enable":1},{"src":"chill","run":1,"enable":1},{"src":"manual","run":1,"enable":1},{"src":"syncauto","run":1,"enable":1}]}` |
| `download_config` | `downloadConfig` | `{"download_log_enable":true,"net_req_cache_disable":true,"bypass_pending_code":true,"net_req_cache_disable_for":[23,24],"noise_download":true,"new_save_image":true,"new_download":true,"enable_user_agent":true,"allow_tracking_download_issues":true,"net_log_enable":false,"net_log_enable_for":[23,24],"err_code_bypass_pending":[],"err_code_apply_pending":[]}` |
| `download_domains_swap` | — | — |
| `download_old_media` | `downloadOldMedia` | `{"dispose":true}` |
| `download_quota` | `downloadQuota` | `{"profiles":[{"id":"191","src":"auto","max_req":10,"max_byte":4194304,"min_reset_time":60000},{"id":"192","src":"auto","max_req":10,"max_byte":3145728,"min_reset_time":60000},{"id":"181","src":"auto","max_req":10,"max_byte":4194304,"min_reset_time":60000},{"id":"182","src":"auto","max_req":10,"max_byte":4194304,"min_reset_time":60000}],"ticks":[{"src":"auto","min_tick_ts":10000}],"enable":true}` |
| `download_resp_cache` | `downloadRespCache` | `{"enable":true,"rootFolder":"resp_cache","validateConfigs":[{"status":404,"maxRetryTime":120000}],"statusConfigs":[{"status":404,"size":200000,"errorRate":0.001,"partition":4,"transBatch":2,"freePartition":2,"version":1,"failCache":true}]}` |
| `download_src` | `downloadSrc` | `{"enable_download_src":true,"enable_total_src_log":true,"enable_conv_detail_src_log":true}` |
| `dr_config` | `dr_config` | `{"supportMaintenance":1,"bucketId":21}` |
| `e2ee` | `e2ee` | `{"debug":{"enable":false,"show_msg_src":false,"show_indicator_video_v2":false,"can_disable":false,"make_decrypt_failed_1_time":false,"make_decrypt_failed_duplicated_1_msg":false,"make_decrypt_failed_duplicated_all_msg":false,"make_decrypt_failed_over_msg_key_group_1_msg":false,"make_decrypt_failed_all_times":false,"make_send_decrypt_failed_1_time":false,"make_send_decrypt_failed":false,"make_timeout_init_e2ee":false,"init_fail":false,"make_reinit_fail":false,"make_socket_timeout":false,"persist":{"fail_get_list":0,"init_subdevice_fail":0,"get_list_slow":0,"queue_e2ee_overflow":0,"skip_upload_bunlde":0,"make_change_new_device":0,"make_delay_notify_change_session":0,"make_delay_verify_iid_v2":0,"make_decrypt_fail_with_errC":0,"make_delay_process_msg":0,"enable":0},"force_enable_init_e2ee_group":false,"make_slow_handle_ack_delete_key":false,"enable_debug_session":false,"make_slow_pull_offline":false,"maked_conv_off_status_e2ee":"","make_slow_update_session_to_db":false,"make_get_bundle_err":0},"allow_stranger_chat":false,"enable":true,"skip_on_disabled":true,"force_reset":false,"enable_group":false,"enable_beta":true,"max_retry_reinit":3,"max_retry_submit":10,"max_retry_get_list":10,"max_retry_int_subdevice":10,"max_retry_get_id":10,"max_initialize_retry_point":3,"timeout_ack_failed":30000,"time_to_reinitialize":600000,"hide_icon_convitem":true,"hide_icon_header":true,"err_stop_retry":[112,-69],"bundle_err_stop_retry":[-2,-3,-5,-7,-9.1,-9.2,-9.3,-9.4,-9.5,-9.6],"sticker_suggestion":1,"client_sticker_suggestion":1,"max_cache_recent_msgids":2000,"force_renew_v2":true,"respone_fail_init_new_device":true,"preview_google_drive":false,"verify_msg_v2":true,"resend_v2":true,"resend_v3":true,"priority_offline_session":false,"priority_offline_msg":true,"compare_resend_dtm":true,"skip_wrong_iid_msg":false,"acceptable_delay_time":300000,"acceptable_time_clean_up_expired_hash":60000,"check_old_key_store_reinit":false,"should_renew_key_after_restore_backup":true,"learn_more":{"what_is_it":"https://help.zalo.me/huong-dan/chuyen-muc/nhan-tin-va-goi/ma-hoa-dau-cuoi-bao-mat-toi-uu-cho-tro-chuyen","how_to_upgrade":"https://help.zalo.me/huong-dan/chuyen-muc/nhan-tin-va-goi/cach-nang-cap-ma-hoa-dau-cuoi-cho-tro-chuyen","troubleshoot_upgrade":"https://help.zalo.me/huong-dan/chuyen-muc/nhan-tin-va-goi/huong-dan-xu-ly-loi-khi-nang-cap-ma-hoa-dau-cuoi","troubleshoot_usage":"https://help.zalo.me/huong-dan/chuyen-muc/nhan-tin-va-goi/xu-ly-cac-loi-trong-tro-chuyen-da-nang-cap-ma-hoa-dau-cuoi"},"max_retry_db_get":{"max_retry":4,"threshold":6},"max_retry_db_update":{"max_retry":3,"threshold":4},"max_retry_db_delete":{"max_retry":2,"threshold":4},"group":{"max_members":10,"min_members":1,"show_detail_error_when_init":true,"can_disable":true,"run_check_session_store_migration_v3":true,"can_enable_right_in_creation_step":false,"deactivated":true,"ack_process_init_session_fail":true,"ack_group_sessions_stale":false,"enable_check_sessions_stable_ts":true,"enable_pairwise_session_refreshing":true,"pairwise_session_version":2,"enable_grlist_sync_check":false,"ack_deliver_fail_over_2000_msg":false,"force_reinit_on_receive_ack_deliver_fail":false,"subscribe_event_group_info_change":true,"timeout_force_sync_group_list":604800000,"validate_checksum":true,"can_retry_db_get":true,"can_retry_db_update":true,"verify_mismatch_on_fetch_list_from_server":true,"verify_mismatch_on_ctrl_event":true,"mismatch_ctrl_event_time_gap":180000,"clear_session_after_disabled":false,"auto_downgrade":false,"auto_downgrade_limit_per_session":50,"debug":{"enable":false,"msgs_number_put_out_of_order":5,"make_out_of_order":false,"make_slow_get_compatibles":false,"make_decrypt_init_session_fail":false,"make_slow_process_session_group":false}},"enable_wasm":true,"max_resize_photo":10,"force_upload_normal":false,"enable_video_version_2":false,"enable_validation_on_runtime":true,"sessions_cache_size_limit":1000,"opkeys_cache_size_limit":1000,"sessions_number_to_rollout_on_threshold":3,"max_message_keys":10000,"check_lost_broadcast_device":true,"back_to_old_resend_timeout":false,"enable_decrypt_e2esession_url_thumb_link_preview":true,"attach_sender_registration_id":true,"re_encrypt_for_stale_sessions":true,"re_getlist_after_manual_logout":true,"reinit_subdevices_after_manual_logout":true,"check_lost_remote_subdevices_reinit":true,"split_processing_queue_offline_msg_11":false,"portion_size_to_split_queue_offline_msg_11":10,"min_portion_size_to_split_queue_offline_msg_11":6,"clean_keys_of_outdated_iid":true,"get_list_e2ee_with_pagination":true,"check_empty_opkeys_then_submit":true,"use_lastid_check_duplicated":true,"wait_to_save_db_session_prekey":true,"reinit_mismatch_iid":true,"max_old_iid_size":100,"validate_msg_iid_v2":true,"strategy_skip_invalid_iid_v2":1,"pre_check_init_msg_type":true,"build_init_msg_v2":true,"enable_e2ee_qos_monitor":true,"enable_notify_delete_session":true,"invisible":true,"backup_deciphered":true,"backup_deciphered_timeout":1209600000,"backup_deciphered_offline_only":true,"check_session_after_crashed":true,"abnormal_decrypt_init_fail_force_off":false,"abnormal_decrypt_init_fail_recover_new_bundle":true,"abnormal_decrypt_init_fail_recover_new_bundle_period":604800000,"decrypt_batching_offline_11":true,"decrypt_batching_limit_cipher_counter":3000,"decrypt_limit_batching_cipher_address":50,"decrypt_batching_time_window_expected":600000,"libsignal_control_session_state":true,"default_11":false,"default_11_autoretry_incompatible":1209600000,"default_11_autoretry_timeout":86400000,"default_11_sendmsg_autoretry":true,"default_11_allow_downgrade":true,"default_11_allow_unencrypted_msg":true,"default_e2ee_characteristic":true,"keep_e2ee_when_receive_normal_msg":false,"should_reinit_after_localstorage_corrupted":true,"default_11_banner":{"enable":false,"frequency":7,"remind":3},"check_subdevice_stale":false,"max_retry_resend":0}` |
| `ecard_config` | `ecard_config` | `{"render_new_birthday_card":1}` |
| `emoji_browser_version` | `emoji_browser_version` | `103` |
| `emoji_sizing` | `emoji_sizing` | `1` |
| `enable_abort_poll_timeout` | `enable_abort_poll_timeout` | `true` |
| `enable_add_identity` | `enable_add_identity` | `true` |
| `enable_add_staredDttm_logic` | `enable_add_staredDttm_logic` | `true` |
| `enable_archived_chat` | `enable_archived_chat` | `1` |
| `enable_auto_hide_scrollbar_conv_list` | `enable_auto_hide_scrollbar_conv_list` | `false` |
| `enable_auto_submit_log` | `enableAutoSubmitLog` | `false` |
| `enable_avatar_log` | `enable_avatar_log` | `0` |
| `enable_avatar_v2` | `enable_avatar_v2` | `0` |
| `enable_blacklist_mention` | `enable_blacklist_mention` | `true` |
| `enable_board_suggestion` | `enable_board_suggestion` | `true` |
| `enable_busy_mode` | `enable_busy_mode` | `0` |
| `enable_cache_fss` | `enable_cache_fss` | `1` |
| `enable_cache_parse_link_fail` | `enable_cache_parse_link_fail` | `0` |
| `enable_check_storage` | `enable_check_storage` | `true` |
| `enable_close_mute_banner` | `enable_close_mute_banner` | `0` |
| `enable_conversation_settings` | `enable_conversation_settings` | `1` |
| `enable_emoji_link` | `enable_emoji_link` | `0` |
| `enable_feature_alias` | `enable_feature_alias` | `1` |
| `enable_feature_alias_v2` | `enable_feature_alias_v2` | `true` |
| `enable_fetch_history_message` | `enableFetchHistoryMessage` | `false` |
| `enable_file_global_search` | `enableFileGlobalSearch` | `true` |
| `enable_file_photo_send_msg_photo` | `enable_file_photo_send_msg_photo` | `1` |
| `enable_file_preview` | `enable_file_preview` | `1` |
| `enable_file_preview_google` | `enable_file_preview_google` | `1` |
| `enable_file_preview_microsoft` | `enable_file_preview_microsoft` | `1` |
| `enable_get_all_process` | `enable_get_all_process` | `0` |
| `enable_get_last_seen` | `enable_get_last_seen` | `1` |
| `enable_get_size_video` | `enable_get_size_video` | `0` |
| `enable_global_search_phase_2` | `enable_global_search_phase_2` | `0` |
| `enable_group_call_entry_for_group` | `enable_group_call_entry_for_group` | `100` |
| `enable_group_call_for_user` | `enable_group_call_for_user` | `1` |
| `enable_group_photo_skeleton` | `enable_group_photo_skeleton` | `1` |
| `enable_guggy` | `enable_guggy` | `0` |
| `enable_hi_group_member` | `enable_hi_group_member` | `false` |
| `enable_hidden_chat` | `enable_hidden_chat` | `true` |
| `enable_history_message` | `enableHistoryMessage` | `true` |
| `enable_idle_reload` | `enable_idle_reload` | `1` |
| `enable_image_link_preview` | `enable_image_link_preview` | `false` |
| `enable_image_retry` | `enableImageRetry` | `true` |
| `enable_interval_log` | `enable_interval_log` | `1` |
| `enable_invite_pc` | `enableInvitePc` | `1` |
| `enable_ipc_call` | `enable_ipc_call` | `true` |
| `enable_log_search` | `enable_log_search` | `false` |
| `enable_mac_call` | `enable_mac_call` | `0` |
| `enable_mediastore_toolkit_banner` | — | — |
| `enable_memory_monitor` | `enableMemoryMonitor` | `false` |
| `enable_multi_recall` | `enable_multi_recall` | `true` |
| `enable_multi_select` | `enableMultiSelect` | `true` |
| `enable_multi_window` | `enable_multi_window` | `1` |
| `enable_new_fss` | `enable_new_fss` | `1` |
| `enable_new_privacy_stranger_chat` | `enable_new_privacy_stranger_chat` | `1` |
| `enable_new_rule_sort_recent_search` | `enable_new_rule_sort_recent_search` | `0` |
| `enable_new_ui_stranger` | `enable_new_ui_stranger` | `1` |
| `enable_nretry` | `enable_nretry` | `true` |
| `enable_optimize_search` | `enable_optimize_search` | `1` |
| `enable_parse_msg_v2` | `enable_parse_msg_v2` | `1` |
| `enable_parse_patterns` | `enable_parse_patterns` | `1` |
| `enable_pc_info` | `enable_pc_info` | `0` |
| `enable_pc_usage` | `enable_pc_usage` | `0` |
| `enable_photo_editor` | `enable_photo_editor` | `0` |
| `enable_photoviewer_popup` | `enable_photoviewer_popup` | `0` |
| `enable_photoviewer_reaction` | `enable_photoviewer_reaction` | `1` |
| `enable_photoviewer_timeline_slider_wheel` | `enable_photoviewer_timeline_slider_wheel` | `1` |
| `enable_polling_last_action` | `enablePollingLastAction` | `true` |
| `enable_polling_last_action_ctrl` | `enablePollingLastActionCtrl` | `true` |
| `enable_popup_copy_excel` | `enable_popup_copy_excel` | `true` |
| `enable_preview_device_incall` | `enable_preview_device_incall` | `1` |
| `enable_preview_device_outgoing` | `enable_preview_device_outgoing` | `1` |
| `enable_privacy_settings` | `enable_privacy_settings` | `true` |
| `enable_profile_loading` | `enable_profile_loading` | `1` |
| `enable_reminder_1_1` | `enable_reminder_1_1` | `1` |
| `enable_report_user` | `enable_report_user` | `1` |
| `enable_search_stranger` | `enable_search_stranger` | `1` |
| `enable_search_top_result` | `enable_search_top_result` | `0` |
| `enable_sticker_hi` | `enable_sticker_hi` | `true` |
| `enable_stranger_alias` | `enable_stranger_alias` | `1` |
| `enable_suggest_file` | — | — |
| `enable_suggest_link` | — | — |
| `enable_suggest_photo` | — | — |
| `enable_swap_domain` | `enable_swap_domain` | `true` |
| `enable_syc_settings` | `enable_syc_settings` | `1` |
| `enable_sync` | — | — |
| `enable_sync_pinned` | `enable_sync_pinned` | `true` |
| `enable_typing_icon` | `enable_typing_icon` | `false` |
| `enable_undo` | `enable_undo` | `1` |
| `enable_unread_v2` | `enable_unread_v2` | `true` |
| `enable_unstable_connecting` | `enable_unstable_connecting` | `0` |
| `enable_upload_file_w_thumb` | `enable_upload_file_w_thumb` | `1` |
| `enable_warning_stranger` | `enable_warning_stranger` | `1` |
| `events_tracking` | `events_tracking` | `{"user_activation":{"enabled":true,"new_user_profile":{"enabled":true},"key_action":{"enabled":true},"new_device_user_activation":{"enabled":true},"duration":7,"enable_log_only_for_new_user":false,"activation_targets":{"sync_success":0,"send_msg":50,"received_msg":50,"clear_unread":10}}}` |
| `export_import` | `export_import` | `{"is_enable":true,"max_size_media":500000000000,"information_url":""}` |
| `file` | `file` | `{"enable_indicator":1,"enable_indicator_ver":1,"enable_cloud":0,"enable_cloud_ver":1,"cloud_limit_11":-1,"cloud_limit_group":-1,"cloud_limit_send2me":104857600,"min_shelf_life_11":7,"min_shelf_life_group":7,"min_shelf_life_send2me":7,"never_expire_11":0,"never_expire_group":0,"never_expire_send2me":0,"enable_downloaded_manual":true,"enable_downloaded_manual_ver":1,"enable_downloaded_auto":0,"enable_downloaded_auto_ver":1,"enable_expired":true,"enable_expired_ver":1,"file_indicator_ab_test":0,"time_to_show_download_err":10000,"win_open_item_regex":"((\\$|\\@)\\(.*\\))|(^.*\\.(([aA][pP][pP][lL][iI][cC][aA][tT][iI][oO][nN])|([bB][aA][tT])|([cC][mM][dD])|([cC][oO][mM])|([cC][pP][lL])|([eE][xX][eE])|([gG][aA][dD][gG][eE][tT])|([hH][tT][aA])|([iI][nN][fF])|([jJ][aA][rR])|([jJ][sS])|([jJ][sS][eE])|([lL][nN][kK])|([mM][sS][cC])|([mM][sS][hH])|([mM][sS][hH]1)|([mM][sS][hH]1[xX][mM][lL])|([mM][sS][hH]2)|([mM][sS][hH]2[xX][mM][lL])|([mM][sS][hH][xX][mM][lL])|([mM][sS][iI])|([mM][sS][pP])|([pP][iI][fF])|([pP][iI][fF][sS])|([pP][sS]1)|([pP][sS]1[xX][mM][lL])|([pP][sS]2)|([pP][sS]2[xX][mM][lL])|([pP][sS][cC]1)|([pP][sS][cC]2)|([sS][cC][fF])|([sS][cC][rR])|([vV][bB])|([vV][bB][eE])|([vV][bB][sS])|([wW][sS])|([wW][sS][cC])|([wW][sS][fF])|([wW][sS][hH])|([rR][eE][gG])|([uU][rR][lL]))$)|([\\u2019\\u2018\\u201a\\u201b\\u275b\\u275c\\u275f\\u201c\\u201d\\u201e\\u201c\\u201f\\u201d\\u2022\\u275d\\u275e\\u2e42\\u301d\\u301e\\u301f\\uff02\\u2013\\u2014\\u2212\\u2022\\uFFFD]+)","enable_file_eecx":true,"enable_switch_open_folder":false,"enable_text_file_cache":1,"file_indicator":0,"time_expire_file":7}` |
| `file_indicator` | — | — |
| `file_photo_limit` | `file_photo_limit` | `{"maxInputWidth":30000,"size":5242880,"maxInputHeight":30000,"enable_upload_original":1,"width":2560,"maxLoopResize":100,"recentW":720,"height":2560,"recentH":720,"maxInputSize":104857600,"enableCompressPhoto":true,"limitCopyPhotoToClipboard":{"enable":1,"limitDimension":8000},"keep_original_param_when_forward":1,"enable_new_endpoint_upload_photo":0,"free_user_can_reupload_original":0,"resolutionLimit":{"hd":{"width":2560,"height":2560,"size":5242880,"maxInputWidth":30000,"maxInputHeight":30000,"maxInputSize":104857600},"original":{"width":30000,"height":30000,"size":104857600,"maxInputWidth":30000,"maxInputHeight":30000,"maxInputSize":104857600}},"maxInputPreviewMemory":838860800}` |
| `forward_conv_limit` | `forward_conv_limit` | `100` |
| `forward_messages` | `forward_messages` | `{"enable_new_forward":1,"enable_forward_video_rotation":1,"forward_contact":{"enable_get_guid_from_cached":1,"enable_get_guid_from_profile":1,"enable_attach_guid_for_non_e2ee":1,"enable_attach_guid_for_e2ee":1},"enable_forward_stranger":1,"enable_tracking_log":0,"enable_head_req":0}` |
| `friend` | `friend` | `"1793873826873"` |
| `fw_rtf_by_send` | `fw_rtf_by_send` | `0` |
| `get_last_message_start_after_lock` | `get_last_message_start_after_lock` | `43200000` |
| `get_last_seen_onlogin` | `get_last_seen_onlogin` | `false` |
| `global_search` | `global_search` | `{"exact_length_bonus":0,"accent_bonus_rule_v2":0,"accent_bonus":4000,"min_matching_bonus":0,"split_word_extend":0,"none_accent_full_match":0,"interaction_bonus":[{"time":86400000,"bonus":3500},{"time":259200000,"bonus":3000},{"time":604800000,"bonus":2500}]}` |
| `global_search_rule` | `global_search_rule` | `{"time_range_first_load":604800000,"time_range_load_more":2592000000}` |
| `groupPoll` | `groupPoll` | `{"enable_poll_v3":1,"max_question_length":200,"max_option_length":120,"min_options":2,"max_options":30,"bonus_due_date":900000,"max_users_display":6,"display_options_msg_poll":3,"max_info_msg_display":2,"max_info_msg_lines":2,"max_info_msg_question_length":50,"max_preview_members":2,"enable_anonymous_vote":1,"enable_hide_vote_preview":1}` |
| `group_call_max_users` | `group_call_max_users` | `8` |
| `group_privacy` | `group_privacy` | `{"invitation_box":{"enable":1,"paging_limit":12,"mcount_item":10,"max_auto_focus_time":86400000,"learn_more":"https://help.zalo.me/huong-dan/chuyen-muc/tro-chuyen-nhom/khi-nao-ban-nhan-duoc-loi-moi-vao-nhom-va-cong-dong/"},"community_zbiz":{"base_pkg":1,"base_pkg_name":"zBusiness Basic","advanced_pkg":3,"learn_more":"https://help.zalo.me/huong-dan/chuyen-muc/tro-chuyen-nhom/lam-gi-khi-khong-the-nang-cap-nhom-len-cong-dong/","learn_more_create":"https://help.zalo.me/huong-dan/chuyen-muc/tro-chuyen-nhom/lam-gi-khi-khong-the-tao-cong-dong/","service_agreement":"https://business.zbox.vn/service-agreement"},"hide_member":{"enable":1,"learn_more":"https://help.zalo.me/huong-dan/chuyen-muc/tro-chuyen-nhom/quyen-xem-danh-sach-thanh-vien/"},"community":{"learn_more":"https://help.zalo.me/huong-dan/chuyen-muc/tro-chuyen-nhom/tro-chuyen-cong-dong-them-ket-noi-them-quyen-rieng-tu/","enable_ui":1,"enable_upgrade":1,"enable":1,"grouptype_threshold":100,"minimum_warn_upgrade":100,"onboard_tip_time":5000,"max_onboard_times":1},"leave_group_silently":{"enable":1},"big_group":{"minimum_member_total_for_big_group":100},"max_expand_member":1000,"group_created_by_oa":1,"quota":{"max_group":1000,"learn_more":""}}` |
| `group_settings` | `group_settings` | `{"mcount":100,"replace_gi_by_glist":1,"enable_new_handle_group_event":0,"group_info_valid_time":259200000,"check_gi_by_event_queue":0,"write_db_time":30000,"delete_conv_by_verify_gr_list":0,"time_expired_data_group_list":1800000}` |
| `group_topics` | `group_topics` | `{"enable":1,"data_expired_time":172800000,"pin_limit":3,"enable_pin_limit":1,"fetch_on_open_board_details":1,"enable_upcoming_event":0,"unread_expired_time":86400000,"using_new_api":1,"using_dev_api":0}` |
| `haspcclient` | `haspcclient` | `1` |
| `icon_event` | `icon_event` | `null` |
| `id` | — | — |
| `iframe_setting` | `iframe_setting` | `{"restructure_link":true}` |
| `image_resizer` | `image_resizer` | `{"enable_libvips_macos":false,"enable_libvips_win32":1}` |
| `important_msg_config` | `important_msg_config` | `{"noti_duration":600000,"gap_time":120000,"gap_send_11":0,"disable":0,"limit_send_11":30}` |
| `inapp_payment` | — | — |
| `index_queue_search` | `index_queue_search` | `{"slow_delay":1000,"slow_batch":100,"fast_delay":100,"fast_batch":100,"onSentry":0,"stopFallbackIndex":0,"readyToInsertCount":2}` |
| `interval_get_recommendations` | — | — |
| `is_enable_tieqviet` | — | — |
| `kiki_assistant` | `kiki_assistant` | `{"enable":0,"h5_url":"","tooltip_promotion":1,"red_dot_promotion":1,"delay_app_ready_ts":3000,"loading_timeout":30000,"oauth_bypass":{"enabled":1,"csp_detect_timeout":10000}}` |
| `label_cfg` | — | — |
| `lan_friends` | — | — |
| `last_seen_react` | `last_seen_react` | `{"max_thread_get_last_seen":50,"max_batch_get_last_seen":10}` |
| `last_seen_throttle` | — | — |
| `lastseen_evict_queues` | `lastseen_evict_queues` | `["1","7"]` |
| `limit_by_hour` | — | — |
| `limit_child_window` | `limitChildWindow` | `{"max_1gb":0,"max_2gb":2,"max_4gb":4,"max_8gb":8,"max_16gb":12}` |
| `limit_date_promote_suggest` | — | — |
| `limit_friends` | `limitFriends` | `2000` |
| `limit_msg_byte` | `limitMsgByte` | `3000` |
| `limit_msg_check_threshold` | `limitMsgCheckThreshold` | `1500` |
| `limit_stranger_invition` | `limit_stranger_invition` | `100` |
| `list_domain_bypass_cache_parse` | `list_domain_bypass_cache_parse` | `["dev-catalog.zalo.me","catalog.zalo.me"]` |
| `list_friend_online_time` | `list_friend_online_time` | `60000` |
| `list_valid_group_summary` | `list_valid_group_summary` | `{}` |
| `load_msg` | `load_msg` | `{"should_loadmore_group_photo":true,"rule_sort_version":3}` |
| `loading_timeout_no_net` | `loading_timeout_no_net` | `30000` |
| `map_api_key` | `map_api_key` | `"AIzaSyBkeSHnerWPkqVM1-b5kk1xvtol7qAOi-A"` |
| `map_embeded_mode` | `map_embeded_mode` | `1` |
| `mark_unread` | `mark_unread` | `{"lastFetchValid":86400000,"enable":1,"max_remove_retry":5,"limit":50,"max_add_retry":1,"max_fetch_retry":5}` |
| `max_member_group_show_seen` | `max_member_group_show_seen` | `100` |
| `max_member_mention_all` | `maxMemberMentionAll` | `100` |
| `max_msg_per_del` | `max_msg_per_del` | `50` |
| `max_msg_per_recall` | `max_msg_per_recall` | `50` |
| `md5_cache_expiration` | `md5CacheExpiration` | `604800000` |
| `media_config` | — | — |
| `media_info_cache_time` | `media_info_cache_time` | `900000` |
| `media_status` | `mediaStatus` | `{"debug":{"enable":true},"delay_download_automatically":1000,"delay_display_media_status":100,"delay_check_status_inview_chat":0,"delay_check_status_inview_media":200,"enable_file_inview_download":true,"enable_expired_soon":true,"enable_learn_more_btn":true,"enable_media_status":true,"enable_cloud_expired_soon_day":false,"enable_download_option_for_download_status":true,"enable_view_folder_option_for_auto_download":true,"enable_forward_file_using_auto_download":true,"enable_loading_auto_downloading_state":true,"should_verify_url_response":false,"learn_more":"https://help.zalo.me/huong-dan/chuyen-muc/nhan-tin-va-goi/anh-video-file-khong-con-ton-tai/","learn_more_paid":"https://help.zbox.vn/2023/11/23/gioi-han-luu-tru-du-lieu-zcloud/","noncloud":{"limitBigFile":100,"TTLUpper":604800000,"TTLLower":1209600000},"cloud":{"limitBigFile":100,"TTLUpper":604800000,"TTLLower":1209600000},"3/noncloud/expired":{"TTL":1209600000},"3/cloud/expired":{"TTL":1209600000},"actionLogInfo":{"enable_sub_type_1":true,"enable_sub_type_2":true,"enable_sub_type_2_for_photo":true,"time_to_log_photo":"0 */5 * * * *"}}` |
| `media_status_config` | `media_status_config` | `{"icon":{"enable_non_cloud":1,"enable_roll":1,"enable_cloud":1},"text":{"enable_non_cloud":1,"enable_roll":1,"enable_cloud":1}}` |
| `message_conf` | `message_conf` | `{"timeout_show_group":10000}` |
| `metric` | — | — |
| `msg_loader` | `msg_loader` | `{"jump_version":2}` |
| `msg_sync` | `msg_sync` | `{"overflow_queue":[1,"510_0",6,"511_0","513_0","514_0","511_1",16,"510_1"]}` |
| `multi_group_settings` | `multi_group_settings` | `{"max_invite":10,"enable_my_group":1}` |
| `multifw_min_timeout` | `multifw_min_timeout` | `60000` |
| `multifw_timeout_per_uid` | `multifw_timeout_per_uid` | `10000` |
| `mute_banner` | — | — |
| `name_card` | `name_card` | `{"qr_enable":1,"fetch_qr_enable":1,"preview_card_enable":1,"retry_parse_card":0}` |
| `network_service` | `network_service` | `{"doh_default":"https://doh.zconf.me/dns-query"}` |
| `new_file` | `newFile` | `{"maxFileSize":1073741824,"firefoxBlackListMime":["application/pdf"],"webUploadFileE2eSizeLimit":52428800,"webDownloadFileE2eSizeLimit":52428800,"webUploadWasm":1073741824,"webDownloadWasm":1073741824,"webDownloadWorkers":[{"type":"manual","quantity":1},{"type":"dev","quantity":1}],"webDownloadQueues":[{"type":"manual","workers":2},{"type":"dev","workers":2}],"webDiskCacheConfig":{"maxTotalFileSize":26214400,"maxFileSize":524288,"maxSize":50},"recallMsgCleanMode":2,"maxFileGenThumbSize":0,"maxImgContentGenThumb":150,"maxMemCacheSizeGenThumb":300,"maxRetryTimeGenThumb":3,"maxPixelPDF":2174960,"maxObjectInPagePDF":100000,"enable_gen_thumb_pdf":false,"enable_gen_thumb_photo":true,"enable_gen_thumb_office":true}` |
| `new_layout_group_photo_2x2` | `new_layout_group_photo_2x2` | `true` |
| `new_msgcache` | `new_msgcache` | `{"enable":1,"msgs_per_conv":68,"max_conv":1000,"preload":15,"verify_newer_cache":true,"jump_enable_clear_cache":true}` |
| `noti_center_config` | `noti_center_config` | `{"disable":0,"max_other_tab":100,"date_other_tab":3,"entry_position":2,"unread_react_in_conv":1,"show_red_dot":0,"show_tip_v2":1}` |
| `notify_missing_e2ee_message` | `notify_missing_e2ee_message` | `{"enable":false,"conditions":{"min_number_of_e2ee_convs":1}}` |
| `num_file_check_promote` | — | — |
| `num_item_db` | — | — |
| `num_item_kw` | — | — |
| `num_item_show` | — | — |
| `num_quick_view_global_search` | `num_quick_view_global_search` | `{"chat":6,"message":3,"file":3,"topRes":3,"personal":8,"group":6,"oa":3}` |
| `off_message_reaction` | `off_message_reaction` | `false` |
| `off_silent_fetch_group` | `off_silent_fetch_group` | `false` |
| `online_configs` | `online_configs` | `{"is_enable_new":1,"update_on_focus":false,"enable_broadcast":false,"enable_group":false,"enable_typing_broadcast":1,"ignore_action_time":true,"user_active_time":180000,"throtle_update_on_focus":5000,"cancellable_get_online_time":1000,"enable_lost_focus":true,"enable_focus_manager":true,"update_action_interval":60000,"enable_background_tracking":true,"idle_time":180000,"idle_interval_check":10000,"track_inactive_alive_back":true,"enable_deact_on_bg_idle":true,"enable_deact_on_fg_idle":true,"enable_active_deactive_v2":true,"enable_full_bg_tracking":true,"send_active_to_keep_live":false,"send_active_using_socket":true}` |
| `only_admin_chat_setting` | `only_admin_chat_setting` | `{"enable_only_admin_chat_setting":1}` |
| `open_link_config` | `open_link_config` | `{"domains":["nld.com.vn","phunugiadinh.vn","galaxycine.vn","evn.com.vn","sendo.vn","vtv.vn","google.com","diadiemanuong.com","youtu.be","hdsaison.com.vn","zingmp3.vn","baovephapluat.vn","boxiangyx.com","cafeland.vn","foody.vn","thegioididong.com","redsun-iti.com.vn","stulish.com","foodbook.vn","baokhanhhoa.vn","surveymonkey.com","tb.cn","webtretho.com","phatsuonline.com","tapdoandiaocalibaba.com","tintuctrongngay.info","cand.com.vn","taobao.com","ead.me","facebook.com","google.com.vn","anninhthudo.vn","cafebiz.vn","24h.com.vn","eva.vn","dienmayxanh.com","chotot.com","890m.com","instagram.com","ikara.co","danviet.vn","npc.com.vn","nguoichinhtruc.com","vtc.vn","fbsbx.com","homecredit.vn","soha.vn","shb.com.vn","viber.com","zalo.me"],"enable":1,"nretry":1000000,"preCheckDomains":{"enable":1,"domains":["media.zalo.me","rd.zapps.vn"]},"zim":"P6Brnjb_M0zb1F_4tnPOO6O0uwhQ5rXZP0lZgZ0v","jumpUrl":"https://redirect.zalo.me/v3/verifyv2/pc?token=P6Brnjb_M0zb1F_4tnPOO6O0uwhQ5rXZP0lZgZ0v&continue=","time_refetch":3600}` |
| `operation_frame_list` | `operation_frame_list` | `{"group":[],"personal":[]}` |
| `parse_control` | — | — |
| `parse_link` | — | — |
| `pc_usage_interval` | `pc_usage_interval` | `3600000` |
| `perf_max_size_img_cache` | `perf_max_size_img_cache` | `500` |
| `perma_ban_status_code` | `perma_ban_status_code` | `{"enable_zip_manager":0,"enable_dimension_helper":0,"zip_manager_code":[],"dimension_helper_code":[]}` |
| `phonebook` | `phonebook` | `1788541003` |
| `photoviewer_num_loadmore` | `photoviewer_num_loadmore` | `20` |
| `photoviewer_popup_mode` | `photoviewer_popup_mode` | `2` |
| `pin_setting` | `pin_setting` | `{"enablePinOther":false,"maxPinOther":5}` |
| `polling_conf` | `polling_conf` | `{"retry_set_message":3,"reset_max_diff":1000,"force_save_curr_id":0}` |
| `predownload_config` | `predownloadConfig` | `{"enable_predownload_video_size":false,"predownload_max_files":100,"predownload_max_file_size":31457280,"predownload_max_video_size":104857600}` |
| `preload_img_in_chat` | `preload_img_in_chat` | `false` |
| `prioritize_share_send2me` | — | — |
| `queue_modified` | `queue_modified` | `{"undo":1,"del_everyone":1,"del":1,"timeout":60000,"max_undo":500,"max_del_batch":500,"max_vanish":50,"debug":{"useMsgId":false},"max_chunk_size":2000,"delay_chunk_time":5000}` |
| `quote_icon` | `quote_icon` | `0` |
| `range_request` | `range_request` | `{"enable_zip_manager":1,"enable_dimension_helper":1,"zip_manager_valid_time":86400000,"dimension_helper_valid_time":86400000}` |
| `recent_photo` | `recent_photo` | `{"enableFeature":false,"maxImageClipboard":50}` |
| `recent_search` | `recent_search` | `{"is_enable":1,"num_on_ui":5,"num_on_db":20,"max_fr_request":3,"max_fr_suggest":3,"fr_mode":-1}` |
| `redundant_res_cleanup` | `redundant_res_cleanup` | `{"schedule_delay":31536000000,"enable_del":1,"enable_for_zcloud":1,"enable":0,"version":0,"del_stat_cache":1,"collect_gen_jpg":1,"batch_size":200,"batch_delay":3000,"batch_conv":5,"batch_size_idle":500,"batch_delay_idle":1000,"start_delay":600000,"retry_delay":86400000,"expired_cache_time":604800000,"enable_use_scan_messages_v2":0}` |
| `register_catalog` | — | — |
| `report_v2` | `report_v2` | `{"enable":1,"max_msg_attach":10,"max_img_attach":5,"max_img_dim":2560,"max_img_file_size":1048576,"resize_img_quality":0.5,"resize_img_percent":0.5,"resize_img_dim":640,"allow_attach":1,"max_other_reason_length":1000,"learn_more":"https://help.zalo.me/huong-dan/chuyen-muc/bao-mat-va-rieng-tu/bao-xau-tai-khoan-zalo/","term_of_service":"https://zalo.me/zalo/dieukhoan/","reason_obj":{"person":[1,2,3,0],"group":[1,2,3,0]},"reason_other_id":0}` |
| `retry_config` | `retryConfig` | `{"setting_feature":1,"time_retry_default":1800000,"time_retry_long":86400000,"limit_sending_msg":1000,"limit_sending_time":86400000,"move_bottom_failed_msg":1,"max_page":5,"max_retry_count":3,"enable_add_missing_retry":1,"max_err_network_retry_count":99,"max_normal_request_retry_count":1,"max_waiting_check_status_time":60000,"retry_strategy":null}` |
| `retry_err_no_network` | `retry_err_no_network` | `1` |
| `retry_err_unknown_exception` | `retry_err_unknown_exception` | `true` |
| `retry_send_seen` | `retrySendSeen` | `{"timeout":259200000,"not_retry":[114,-69]}` |
| `retry_upload_file_configs` | — | — |
| `roll_msg` | `roll_msg` | `{"enable":false,"enable_roll_db":false,"enable_roll_log":true,"delay_init":300000,"expired_time":1209600000,"batch_size":100,"wait_time":5000,"next_round_time":43200000,"keep_conv_in_conv_list_when_roll_data":0}` |
| `rule_contact_search` | `rule_contact_search` | `{"rule_weight_t10_2021":1,"decay_hit_count":0}` |
| `scandownload_config` | — | — |
| `search_in_conv` | `search_in_conv` | `{"new_rule_prefix_match":0,"btn_jump_minimode":0}` |
| `search_message` | `search_message` | `{"in_chat":{"enable_matching_enhance":1,"enable_ux_enhance":1,"max_load_more":50,"bonus_load_more":5,"max_init_load":5,"max_preview_len":70,"bonus_preview_len":30,"max_preview_file_len":35,"max_kw_len":100,"promote":1,"promote_auto_close":10000,"promote_delay":3000},"global":{"enable_matching_enhance":1,"enable_ux_enhance":1,"max_preview_len":65,"bonus_preview_len":25,"max_preview_file_len":35,"max_prefix_len":15},"bubble":{"enable_ux_enhance":1}}` |
| `search_mutual_group_member` | `search_mutual_group_member` | `1` |
| `search_phone` | `search_phone` | `{"limit_by_hour":1}` |
| `send_bankcard_config` | `send_bankcard_config` | `{"showToolTip":0,"pagingLimit":20,"cacheTimeGetBankCard":30,"showReddot":0,"isActive":1}` |
| `send_seen_subtab_retry` | — | — |
| `sentry_config` | — | — |
| `server_maintenance` | — | — |
| `sgm_anim` | — | — |
| `sharefile` | — | — |
| `shortcut_suggest_config` | `shortcut_suggest_config` | `{"gif_display":20,"guggy_display":9,"sticker_display":20}` |
| `should_attach_extraMsgProp` | `should_attach_extraMsgProp` | `1` |
| `should_force_genlist_conv` | `should_force_genlist_conv` | `true` |
| `show_confirm_del` | `show_confirm_del` | `1` |
| `show_onboarding` | `showOnboarding` | `0` |
| `show_sticker_suggest_hint` | `show_sticker_suggest_hint` | `0` |
| `show_warning_ssl_date_error` | — | — |
| `size_conversion_factor` | `size_conversion_factor` | `1024` |
| `sl_upload_log` | — | — |
| `socket` | `socket` | `{"offline_monitor":{"enable":true},"ping_http_level":1,"submit_wss_log":false,"enable_chat_socket":true,"enable_ctrl_socket":true,"decouple_queue_chat":true,"decouple_queue_ctrl":true,"disable_lp":true,"kickout_on_max":true,"skip_message_on_swap":true,"offline_timeout":480000,"ping_interval":180000,"timeout_authen":15000,"interval_webworker":true,"sync_id_from_lp":true,"sync_id_to_lp":true,"sync_ctrl_id_from_lp":true,"sync_ctr_id_to_lp":true,"init_lastid_lp":true,"init_lastid_ctrl_sock":true,"queue_actionid_map":{"510_0":"1","511_0":"6","510_1":"15","511_1":"16"},"queue_ctrl_actionid_map":{"603_1":"19","604_0":"20","603_0":"4","611_1":"17","611_0":"12","610_1":"18","610_0":"5"},"evict_broadcast_queues":{"lp_msg":[1,6,16],"lp_ctrl":[4,19],"sock_msg":["510_0","511_0","513_0","514_0","515_0","511_1","510_1","710_0","711_0","713_0","715_0"],"sock_ctrl":["603_0","603_1"]},"msg_11_cmds":["501"],"msg_11_offline":["510"],"logout_on_err_kick":true,"retries":{"3000":{"max":8,"times":"<object>"},"3002":{"times":"<object>"},"3003":{"max":13,"times":"<object>"},"4001":{"max":8,"times":"<object>"},"4002":{"times":1000},"5001":{"max":8,"times":"<object>"},"5007":{"max":8,"times":"<object>"},"5008":{"max":5,"times":"<object>"},"5011":{"max":5,"times":"<object>"},"5012":{"max":5,"times":3000},"5013":{"max":5,"times":1000},"5014":{"max":8,"times":"<object>"},"5015":{"max":8,"times":"<object>"},"5017":{"max":8,"times":"<object>"},"internal":{"max":8,"times":"<object>"},"swap_endpoint":{"times":1000},"system_resume":{"times":3000},"manual":{"times":1000},"cannot_reach_server":{"max":8,"times":"<object>"},"no_network":{"times":"<object>"},"interval_check":{"times":1000}},"retries_round_2":{"3000":{"multiple":2000,"max":60000},"4001":{"multiple":2000,"max":60000},"5001":{"multiple":2000,"max":60000},"5007":{"multiple":2000,"max":60000},"5008":{"multiple":2000,"max":60000},"5011":{"multiple":2000,"max":60000},"5012":{"multiple":3000,"max":60000},"5014":{"multiple":2000,"max":60000},"5015":{"multiple":2000,"max":60000},"5017":{"multiple":2000,"max":60000},"default":{"multiple":3000,"max":60000},"internal":{"multiple":2000,"max":60000},"cannot_reach_server":{"multiple":2000,"max":60000}},"retry_noise_factor":0.2,"anomalous_error_kalpha":0.46,"close_and_retry_codes":[5008,5007,3003,3000,5014,4002,5015,5017],"rotate_error_codes":[5011,5012,4001,5008],"establish_timeout":15000,"close_timeout":5000,"reconnect_after_fallback":true,"swap_back_socket_time":900000,"enable_fallback_lp":true,"max_msg_verify":20,"max_pending_verify":200,"timeout_retry_miss":30000,"delay_get_msg":10,"max_msg_get":20,"interval_verify":30000,"reset_endpoint":600000,"e2ee_queues":["513","514","515","713","715"],"offline_msg_e2ee_pc":["513","713"],"offline_session_e2ee_pc":"516","offline_msg_e2ee_web":["515","715"],"offline_session_e2ee_web":"517","offline_init_cmds":["516","517","518","519"],"none_ids_queue":["518","519"],"skip_offline_verify":true,"verify_on_done_offline":true,"ping_max_timeout":300000,"close_socket_before_quit":true,"can_block_socket_polling":true,"debug":{"enable":false},"qosCmd":{"state":97108,"error":97109,"open_duration":97115,"open_success_duration":97116,"max_retry":97117},"max_msg_size":4096}` |
| `sticker_cate_cache_expired_time` | `sticker_cate_cache_expired_time` | `86400000` |
| `sticker_group_max_items_per_row` | `sticker_group_max_items_per_row` | `3` |
| `sticker_post_typing` | `sticker_post_typing` | `{"is_enable":false,"delay_to_close":2000,"mouse_enter_delay_to_close":7000,"mouse_leave_delay_to_close":1000,"is_enable_hold_match_kw":false,"debounce":0}` |
| `sticker_settings` | `sticker_settings` | `{"handle_change_cate_version":1,"handle_update_sticker":1,"update_cate_gap_time":1800000,"handle_fetch_cate":{"percent":50,"delay":3540000,"skip":2}}` |
| `sticker_suggest_delay` | `sticker_suggest_delay` | `0` |
| `sticker_suggest_max_hint_count` | `sticker_suggest_max_hint_count` | `1` |
| `sticker_suggestion_models` | — | — |
| `submit_log_conf` | `submit_log_conf` | `{"enable":1,"enable_send":false,"success_delay":300000,"fail_delay":30000}` |
| `suggest_night_chat` | `suggest_night_chat` | `{"limit_item":3,"enable":false,"start_time":1793898000000,"end_time":1793984400000,"time_refetch":1800000}` |
| `sync_del_msg_everyone` | — | — |
| `sync_del_msg_or_conv` | — | — |
| `sync_recent_search` | `sync_recent_search` | `{"enable_sync":1,"enable_kw":0,"num_item_db":10,"num_item_show":20,"num_item_kw":5}` |
| `tab_banner` | `tab_banner` | `{"time_delay_display_on_next_banner":3600000,"features":{"event_operation":{"priority":"low"},"sync_message":{"suggest_sync":"<object>","resume_sync":"<object>","priority":"high"},"sync_message_v2":{"suggest_sync":"<object>","priority":"high"},"backup_pc_mobile":{"priority":"high"},"web_change_device":{"enable":true,"priority":"low","auto_close":0},"e2ee_announcement":{"priority":"normal"},"trusted_device":{"priority":"normal"}},"enable_single_banner":true,"time_delay_display_on_start_app":0}` |
| `tabbed_global_search_result` | `tabbedGlobalSearchResult` | `true` |
| `task_setting` | `task_setting` | `{"api_timeout":86400000,"enable_feature":0,"media_link":["https://media.zalo.me/detail/4290414774003551493?id=b69623562a13c34d9a02&pageId=4290414774003551493","https://media.zalo.me/detail/4290414774003551493?id=eae675277c62953ccc73&pageId=4290414774003551493","https://media.zalo.me/detail/4290414774003551493?id=a54842894bcca292fbdd&pageId=4290414774003551493"],"user_level":1,"beta":0,"show_suggest_btn":1,"support_page":"","suggest_feature":{"extra_button_msg":0,"chat_bar":1,"tooltip":1,"member_list":1,"receive_button":1},"show_confirm_title":1,"reply_on_notify":0,"show_suggest_receive_btn":1,"range_lv1":5,"range_lv2":10,"total_task_valid_time":86400000,"ui_v2":1,"evil_mode":0,"limit_per_fetch":50,"reset_time":129600000,"verify_time":86400000,"enable_repeat":0,"enable_text_comment":1,"enable_reject":1}` |
| `time_cache_parse_link` | `time_cache_parse_link` | `3600000` |
| `time_cache_user_rank` | `time_cache_user_rank` | `300000` |
| `time_clear_cache_skeleton_group_photo` | `time_clear_cache_skeleton_group_photo` | `90000` |
| `time_delay_track_frecency` | `time_delay_track_frecency` | `5000` |
| `time_enable_edit_undo_msg` | `time_enable_edit_undo_msg` | `120000` |
| `time_promote_suggest` | — | — |
| `timeoutMakeCall2ZRtp` | `timeoutMakeCall2ZRtp` | `0` |
| `tip_icon_event` | `tip_icon_event` | `null` |
| `title_bar` | `title_bar` | `1` |
| `todo_banner_settings` | `todo_banner_settings` | `{"off_banner_1_1":0,"dismiss_time":0,"max_interact_time":0,"enable":0,"version":0,"auto_dismiss":0,"off_indicator":1}` |
| `todo_related` | — | — |
| `transfer_when_login` | `transfer_when_login` | `{"no_missing_message_events_timeout":3000,"time_wait_for_temp_key":3000,"enable_drop_flow":true,"enable_log_events_arrival_time":true,"mobile_transfer_confirm_timeout":3000,"success_banner_when_drop":true}` |
| `typing_name_conf` | `typing_name_conf` | `{"max_length":20,"min_length":1,"separators":[" ","-","_"],"sm_length_in_chat":16,"md_length_in_chat":28}` |
| `ui_ux_transfer_revamp` | `ui_ux_transfer_revamp` | `{"enable_ui_popup_transfer_revamp":true,"enable_ux_confirm_transfer_revamp":false,"enable_ui_banner_suggest_sync_revamp":true,"enable_ui_banners_in_transfer_revamp":true,"enable_sync_idle_mobile_state":true,"enable_force_cleanup_if_mobile_busy_backup":false,"contents":{"suggest_sync_banner":{"description":"<object>","cta":"<object>","title":"<object>"}},"progress_points":{"download":10,"decrypt":20,"restore":70}}` |
| `update_conv_time` | `update_conv_time` | `70000` |
| `update_last_action_time` | `update_last_action_time` | `{"trigger_enable_last_action_time":10,"enable_deliver":1,"msg_cache_limit":500}` |
| `update_recent_media_item` | `update_recent_media_item` | `{"trigger_recent_sticker_point":2,"trigger_recent_gif_point":2,"trigger_recent_guggy_point":0,"trigger_recent_photo_sticker_point":2,"trigger_recent_ai_sticker_point":2,"enable_trigger_recent_sticker_point_e2ee":true}` |
| `upload_file` | `upload_file` | `{"enable_discard_low_priority":1,"check_checksum_when_upload":0,"enable_checksum_chucks":1,"upload_fifo":true}` |
| `upload_file_folder_config` | — | — |
| `upload_file_upload_type_folder` | `upload_file_upload_type_folder` | `1` |
| `upload_thumb_file_time_out` | `upload_thumb_file_time_out` | `2` |
| `use_net_error` | `use_net_error` | `0` |
| `user_activation` | — | — |
| `valid_server_data` | — | — |
| `ver_giphy_cate` | `ver_giphy_cate` | `0` |
| `ver_sticker_cate_list` | `ver_sticker_cate_list` | `6951` |
| `ver_sticker_giphy_suggest` | `ver_sticker_giphy_suggest` | `1623` |
| `video_inline` | `video_inline` | `{"enableFeature":true,"enableAutoplay":false}` |
| `white_list_file` | `white_list_file` | `{"addition":[],"removal":["py","svg"],"reset":false,"enable":true}` |
| `z_banner_config` | `z_banner_config` | `{"zpc-promote-zcloud-001":[{"banner_id":"15845","start_time":0,"expired_time":-1,"shown":1,"last_hided_time":0,"max_impressions":-1,"item":"<object>"}],"zcloud-package-info-zCloud-100-GB":[{"banner_id":"15544","start_time":0,"expired_time":-1,"shown":1,"last_hided_time":0,"max_impressions":-1,"item":"<object>"}],"mycloud-right-sidebar":[{"banner_id":"15551","start_time":0,"expired_time":-1,"shown":1,"last_hided_time":0,"max_impressions":-1,"item":"<object>"}],"mycloud-modal":[{"banner_id":"15550","start_time":0,"expired_time":-1,"shown":1,"last_hided_time":0,"max_impressions":-1,"item":"<object>"}],"business_start_page":[{"banner_id":"14875","start_time":0,"expired_time":-1,"shown":1,"last_hided_time":0,"max_impressions":-1,"item":"<object>","desc":"<object>"}],"zcloud-package-info-zCloud-1-TB":[{"banner_id":"15544","start_time":0,"expired_time":-1,"shown":1,"last_hided_time":0,"max_impressions":-1,"item":"<object>"}],"zcloud-package-info-zCloud-500-GB":[{"banner_id":"15544","start_time":0,"expired_time":-1,"shown":1,"last_hided_time":0,"max_impressions":-1,"item":"<object>"}],"zcloud-package-info-zCloud-10-GB":[{"banner_id":"15544","start_time":0,"expired_time":-1,"shown":1,"last_hided_time":0,"max_impressions":-1,"item":"<object>"}]}` |
| `zcloudOnboarding` | `zcloudOnboarding` | `{"show_beta_mark":false,"enable":true,"popTab":{"zCloudText":{"vi":"zCloud","en":"zCloud"},"myCloudText":{"vi":"Cloud \n của tôi","en":"My Cloud"}}}` |
| `zinstant_dynamic_banner` | `zinstant_dynamic_banner` | `{"zcloud":[{"id":"98a94e25-1bd2-4cec-a91b-2f80708ed6d5","name":"zalopc_zmt_sideview_warning_download_resource","type":"zcloud","whitelistOnly":false,"shown":1,"position":"zcloud-banner-side-tab","item":"<object>","banner_id":"15595","start_time":1793898060000,"expired_time":1793984460000,"last_hided_time":0,"max_impressions":-1}],"service_enabled":{"zcloud":true,"zbusiness":true},"enabled":true}` |

## All `enable_*` identifiers in the config module

```
enable_DFE
enable_DFE_admin
enable_DFE_sender
enable_abort_poll_timeout
enable_active_deactive_v2
enable_add_identity
enable_add_missing_retry
enable_add_staredDttm_logic
enable_add_to_recent
enable_anonymous_vote
enable_api_mini_profile
enable_api_verify_fr_list
enable_archived_chat
enable_async_preparse
enable_auto_gen_group_name
enable_auto_hide_scrollbar_conv_list
enable_auto_submit_log
enable_avatar_log
enable_avatar_v2
enable_background_tracking
enable_backup_db
enable_beta
enable_blacklist_mention
enable_block_cache
enable_block_msg_call_setting
enable_board_suggestion
enable_break_folder_db_as_session
enable_broadcast
enable_busy_mode
enable_cache_fss
enable_cache_parse_link_fail
enable_change_password
enable_chat_red_dot
enable_chat_socket
enable_chat_tag
enable_check_queue_ctrl
enable_check_sessions_stable_ts
enable_check_storage
enable_check_thumb_size
enable_checksum_chucks
enable_close_mute_banner
enable_cloud
enable_cloud_expired_soon_day
enable_cloud_ver
enable_con_rt
enable_contact_tab_v2
enable_content_notify_off_login_password
enable_conv_detail_src_log
enable_conversation_settings
enable_ctrl_socket
enable_de_noise_in_forward
enable_deact_on_bg_idle
enable_deact_on_fg_idle
enable_debug_session
enable_decrypt_e2esession_url_thumb_link_preview
enable_delete_on_filter
enable_delete_on_filter_minimum_overtime
enable_delete_quote_on_filter
enable_deliver
enable_dimension_helper
enable_discard_low_priority
enable_discovery
enable_dom_cache
enable_download
enable_download_native
enable_download_option_for_download_status
enable_download_src
enable_download_thumb
enable_download_url
enable_download_web
enable_downloaded_auto
enable_downloaded_auto_ver
enable_downloaded_manual
enable_downloaded_manual_ver
enable_e2ee
enable_e2ee_qos_monitor
enable_edit
enable_emoji_link
enable_encrypt_param
enable_expired
enable_expired_soon
enable_expired_ver
enable_fake_event_banner
enable_fallback_lp
enable_feature
enable_feature_alias
enable_feature_alias_v2
enable_fetch_history_message
enable_file_eecx
enable_file_global_search
enable_file_inview_download
enable_file_photo_send_msg_photo
enable_file_preview
enable_file_preview_google
enable_file_preview_microsoft
enable_file_store_comp
enable_flow_v2
enable_focus_manager
enable_force_cleanup_if_mobile_busy_backup
enable_forward_file_using_auto_download
enable_forward_module
enable_full_bg_tracking
enable_gen_preview_thumb
enable_gen_thumb
enable_gen_thumb_office
enable_gen_thumb_office_mac
enable_gen_thumb_pdf
enable_gen_thumb_photo
enable_get_all_process
enable_get_conv_purpose
enable_get_last_seen
enable_get_last_seen_v2
enable_get_size_video
enable_global_search_phase_2
enable_grlist_sync_check
enable_group
enable_group_call_entry_for_group
enable_group_call_for_user
enable_group_photo_skeleton
enable_group_profile
enable_guggy
enable_handle_69
enable_hi_group_member
enable_hidden_chat
enable_hide_vote_preview
enable_history_message
enable_idle_reload
enable_ignore_localpath_file_noise
enable_image_group_batch
enable_image_link_preview
enable_image_retry
enable_improvement
enable_indicator
enable_indicator_ver
enable_interval_log
enable_invite_pc
enable_ipc_call
enable_jump_last_pos
enable_jump_new
enable_jump_unread
enable_keep_msg_info_DFE
enable_kw
enable_learn_more_btn
enable_libvips_macos
enable_libvips_win32
enable_limiter
enable_load_cache
enable_loader_limiter
enable_loading_auto_downloading_state
enable_log
enable_log_search
enable_logout_v2
enable_lost_focus
enable_mac_call
enable_matching_enhance
enable_media_status
enable_mediastore_toolkit_banner
enable_memory_monitor
enable_merge_msg_offline_status
enable_migrate
enable_migration
enable_mini_info
enable_mining_sale_shop
enable_monitor_forward_file_noise
enable_multi_del_msg
enable_multi_recall
enable_multi_select
enable_multi_window
enable_my_group
enable_new_capture
enable_new_domain
enable_new_fss
enable_new_handle_group_event
enable_new_privacy_stranger_chat
enable_new_rule_sort_recent_search
enable_new_ui_stranger
enable_notify_delete_session
enable_nretry
enable_only_admin_chat_setting
enable_optimize_search
enable_pairwise_session_refreshing
enable_parse_msg_v2
enable_parse_patterns
enable_pc_info
enable_pc_usage
enable_photo_editor
enable_photoviewer_popup
enable_photoviewer_reaction
enable_photoviewer_timeline_slider_wheel
enable_pin_limit
enable_poll_v3
enable_polling_last_action
enable_polling_last_action_ctrl
enable_popup_confim_del
enable_popup_copy_excel
enable_predata_import
enable_predownload_file
enable_predownload_video_size
enable_preload_local_hd
enable_prevent_override_hd
enable_preview_device_incall
enable_preview_device_outgoing
enable_privacy_settings
enable_profile_loading
enable_pv_default_zoom
enable_pv_zoom_reset
enable_pv_zoom_slider
enable_recent_file
enable_reject
enable_reminder_1_1
enable_repeat
enable_report_user
enable_resend_lastchunk
enable_retry_no_network
enable_retry_no_network_confirm
enable_retry_timeout
enable_roll_db
enable_roll_log
enable_rule_last_view_friend_req
enable_search
enable_search_perf_log
enable_search_stranger
enable_search_top_result
enable_settings
enable_share_on_timeline
enable_single_banner
enable_soundcloud
enable_sticker_hi
enable_stranger_alias
enable_sub_type_1
enable_sub_type_2
enable_sub_type_2_for_photo
enable_suggest_create_product_1_1
enable_suggest_create_product_group
enable_suggest_file
enable_suggest_link
enable_suggest_mention
enable_suggest_photo
enable_suggest_recent_file
enable_suggestion
enable_swap_domain
enable_switch_open_folder
enable_syc_settings
enable_sync
enable_sync_cloud_first_login
enable_sync_del_conv
enable_sync_del_msg
enable_sync_delete
enable_sync_idle_mobile_state
enable_sync_pinned
enable_text_comment
enable_text_file_cache
enable_thumb_anim_mode
enable_todo
enable_total_src_log
enable_trigger_recent_sticker_point_e2ee
enable_typing_broadcast
enable_typing_icon
enable_ui_banner_suggest_sync_revamp
enable_ui_banners_in_transfer_revamp
enable_ui_popup_transfer_revamp
enable_undo
enable_unread_v2
enable_unstable_connecting
enable_upcoming_event
enable_upload_file_w_thumb
enable_user_agent
enable_ux_confirm_transfer_revamp
enable_ux_enhance
enable_validation_on_runtime
enable_verify_checksum_forward_cross
enable_video_version_2
enable_view
enable_view_folder_option_for_auto_download
enable_warning_stranger
enable_wasm
enable_youtube
enable_zinstant
enable_zip_manager
```

## All `enable_*` identifiers in the whole app.asar

```
enable_DFE
enable_DFE_admin
enable_DFE_sender
enable_abort_poll_timeout
enable_active_deactive_v2
enable_add
enable_add_identity
enable_add_missing_retry
enable_add_staredDttm_logic
enable_add_to_recent
enable_adl_action_log
enable_anonymous_vote
enable_api_mini_profile
enable_api_verify_fr_list
enable_archived_chat
enable_async_preparse
enable_attach_guid_for_e2ee
enable_attach_guid_for_non_e2ee
enable_audio
enable_auto_download_thumb
enable_auto_gen_group_name
enable_auto_hide_scrollbar_conv_list
enable_auto_request_sync_on_log_in_new_device
enable_auto_request_sync_on_missing_msg_event
enable_auto_scan_after_persist_data
enable_auto_submit_log
enable_avatar_log
enable_avatar_v2
enable_background_tracking
enable_backup_db
enable_backup_old_zalo
enable_badge_login_pw
enable_batching_mode
enable_beta
enable_bg_upload
enable_biz_label
enable_blacklist_mention
enable_block_cache
enable_block_msg_call_setting
enable_board_suggestion
enable_break_folder_db_as_session
enable_broadcast
enable_broadcast_msg_file_photo
enable_bt_upgrade_ba
enable_busy_mode
enable_cache_fss
enable_cache_parse_link_fail
enable_calculate_fileThumb
enable_calculate_richThumb
enable_calculate_sticker_as_cache
enable_calculate_voice
enable_calculate_zalo_temp
enable_calculate_zinstant
enable_calculate_zrf
enable_calculate_zrf_without_display
enable_calculate_ztemp
enable_calf
enable_calf_v2
enable_call
enable_cate
enable_change_password
enable_chat_red_dot
enable_chat_socket
enable_chat_tag
enable_check_big_file
enable_check_profile_rank
enable_check_queue_ctrl
enable_check_sessions_stable_ts
enable_check_storage
enable_check_thumb_size
enable_checksum_chucks
enable_clean_cache
enable_clean_other_account_data
enable_cleanup
enable_client
enable_close_mute_banner
enable_cloud
enable_cloud_expired_soon_day
enable_cloud_ver
enable_collect
enable_community
enable_compare_params
enable_con_rt
enable_conf
enable_contact_tab_v2
enable_content_notify_off_login_password
enable_conv_detail_src_log
enable_conversation_settings
enable_convert_mp3
enable_convertible
enable_cpu_usage_recorder
enable_create_community_any_size
enable_cta_in_suggest_upgrade
enable_cta_upgrade
enable_ctrl_socket
enable_custom_sound_on_browser
enable_de_noise_in_forward
enable_deact_on_bg_idle
enable_deact_on_fg_idle
enable_debug_session
enable_decrypt_e2esession_url_thumb_link_preview
enable_del
enable_delete_on_filter
enable_delete_on_filter_minimum_overtime
enable_delete_quote_on_filter
enable_deliver
enable_detail_log
enable_detect_session_by_websocket
enable_dimension_helper
enable_discard_low_priority
enable_discovery
enable_disk_cache
enable_dismiss
enable_dnr
enable_dom_cache
enable_download
enable_download_native
enable_download_option_for_download_status
enable_download_partial_media_e2ee
enable_download_source_without_message
enable_download_src
enable_download_thumb
enable_download_url
enable_download_web
enable_downloaded_auto
enable_downloaded_auto_ver
enable_downloaded_manual
enable_downloaded_manual_ver
enable_drop_flow
enable_e2ee
enable_e2ee_qos_monitor
enable_edit
enable_electron_api
enable_emoji_link
enable_encrypt_param
enable_entry
enable_entry_login_pw
enable_entrypoint
enable_expired
enable_expired_soon
enable_expired_ver
enable_fail_noti_close_time_ms
enable_failed_noti_close_time_ms
enable_failed_ui
enable_fake_event_banner
enable_fallback_lp
enable_feature
enable_feature_alias
enable_feature_alias_v2
enable_features
enable_fetch_foreground
enable_fetch_history_message
enable_file
enable_file_eecx
enable_file_global_search
enable_file_inview_download
enable_file_photo_send_msg_photo
enable_file_preview
enable_file_preview_google
enable_file_preview_microsoft
enable_file_store_comp
enable_file_types
enable_flow_v2
enable_focus_manager
enable_for_oa
enable_for_staging_account
enable_for_stranger
enable_for_user_with_all_sql_db
enable_for_zcloud
enable_force_cleanup_if_mobile_busy_backup
enable_force_expired_photo
enable_force_parse_link_server
enable_forward_file_using_auto_download
enable_forward_module
enable_forward_stranger
enable_forward_video_rotation
enable_fps_drop_monitor
enable_fps_monitor
enable_fps_monitor_v2
enable_friend_suggestion
enable_full_bg_tracking
enable_gen_preview_thumb
enable_gen_thumb
enable_gen_thumb_office
enable_gen_thumb_office_mac
enable_gen_thumb_pdf
enable_gen_thumb_photo
enable_get_all_process
enable_get_conv_purpose
enable_get_guid_from_cached
enable_get_guid_from_profile
enable_get_last_seen
enable_get_last_seen_v2
enable_get_size_video
enable_global_search_phase_2
enable_grlist_sync_check
enable_group
enable_group_call_entry_for_group
enable_group_call_for_user
enable_group_photo_skeleton
enable_group_profile
enable_guggy
enable_handle_69
enable_head_req
enable_head_request
enable_hi_group_member
enable_hidden_chat
enable_hide_vote_preview
enable_history_message
enable_idle_reload
enable_ignore_localpath_file_noise
enable_image_group_batch
enable_image_link_preview
enable_image_retry
enable_improve_by_window_version
enable_improvement
enable_indicator
enable_indicator_ver
enable_interrupt_noti_close_time_ms
enable_interval_log
enable_inview_checksum
enable_inview_log
enable_inview_upload
enable_invite_pc
enable_ipc_call
enable_jump_last_pos
enable_jump_new
enable_jump_unread
enable_jxl_convertible
enable_jxl_upload
enable_jxl_upload_e2ee
enable_keep_msg_info_DFE
enable_kw
enable_learn_more_btn
enable_libvips_macos
enable_libvips_win32
enable_lifecycle_log
enable_limiter
enable_load_cache
enable_loader_limiter
enable_loading_auto_downloading_state
enable_local_msgId_attach
enable_log
enable_log_events_arrival_time
enable_log_metric
enable_log_migration_status
enable_log_only_for_new_user
enable_log_search
enable_log_when_create_file_at_download_path
enable_log_when_create_file_at_temp
enable_logout_v2
enable_lost_focus
enable_mac_call
enable_manage_cache
enable_matching_enhance
enable_media_status
enable_mediastore_toolkit_banner
enable_memory_monitor
enable_merge_msg_offline_status
enable_migrate
enable_migrate_media_to_new_position
enable_migration
enable_mini_info
enable_mining_sale_shop
enable_monitor_forward_file_noise
enable_multi_del_msg
enable_multi_recall
enable_multi_select
enable_multi_site_api
enable_multi_window
enable_my_cloud
enable_my_group
enable_mycloud
enable_native_electron_jxl
enable_new_cap
enable_new_capture
enable_new_domain
enable_new_endpoint_upload_photo
enable_new_folder_to_media
enable_new_forward
enable_new_fss
enable_new_handle_group_event
enable_new_privacy_stranger_chat
enable_new_rule_sort_recent_search
enable_new_ui_stranger
enable_non_cloud
enable_notify_delete_session
enable_nretry
enable_nw_photo_player
enable_oan_others_tab
enable_offload_decode_jxl
enable_offload_encode_jxl
enable_offload_jxl_resize
enable_offload_lipvips_resize
enable_on_platform
enable_on_sync_source_setting
enable_only_admin_chat_setting
enable_optimize_search
enable_other
enable_outer_close_region
enable_pairwise_session_refreshing
enable_parse_link_client
enable_parse_link_receiver
enable_parse_link_sender
enable_parse_link_server
enable_parse_msg_v2
enable_parse_patterns
enable_partial_stat
enable_paused_noti_mobile
enable_pc_info
enable_pc_usage
enable_pcloud_trigger_inview_transfer_key
enable_pcloud_trigger_manual_transfer_key
enable_photo
enable_photo_editor
enable_photoviewer_popup
enable_photoviewer_reaction
enable_photoviewer_timeline_slider_wheel
enable_pin_jxl
enable_pin_limit
enable_poll_v3
enable_polling_last_action
enable_polling_last_action_ctrl
enable_popup_confim_del
enable_popup_copy_excel
enable_predata_import
enable_predownload_community_media
enable_predownload_file
enable_predownload_video_size
enable_preload
enable_preload_data_log
enable_preload_e2ee_log
enable_preload_local_hd
enable_prevent_override_hd
enable_preview_device_incall
enable_preview_device_outgoing
enable_privacy_settings
enable_process_lifecycle
enable_profile_loading
enable_promo_bagde_new
enable_promo_popover
enable_promo_tooltip
enable_pv_default_zoom
enable_pv_zoom_reset
enable_pv_zoom_slider
enable_qos
enable_quota
enable_reaction_v2
enable_recent_file
enable_recorrect_cloud_key
enable_red_banner
enable_reject
enable_reminder_1_1
enable_repeat
enable_report
enable_report_migrate_db
enable_report_user
enable_request_cpu_idle
enable_resend_lastchunk
enable_restore_task
enable_retry_no_network
enable_retry_no_network_confirm
enable_retry_timeout
enable_roll
enable_roll_db
enable_roll_log
enable_rule_last_view_friend_req
enable_save_file_in_media
enable_schema
enable_search
enable_search_perf_log
enable_search_stranger
enable_search_top_result
enable_seen_by_ts
enable_segment_updater
enable_send
enable_send2me
enable_settings
enable_share_on_timeline
enable_show_loading_thumb
enable_single_banner
enable_soundcloud
enable_sticker_hi
enable_stranger_alias
enable_sub_type_1
enable_sub_type_2
enable_sub_type_2_for_photo
enable_submit_perf_qos
enable_submit_qos
enable_success_noti_close_time_ms
enable_success_noti_pcweb
enable_suggest_create_product_1_1
enable_suggest_create_product_group
enable_suggest_file
enable_suggest_link
enable_suggest_mention
enable_suggest_noti_close_time_ms
enable_suggest_photo
enable_suggest_recent_file
enable_suggestion
enable_swap_domain
enable_switch_open_folder
enable_syc_settings
enable_sync
enable_sync_cbs_cross_plf
enable_sync_cloud_first_login
enable_sync_del_conv
enable_sync_del_msg
enable_sync_delete
enable_sync_download_action_v2
enable_sync_idle_mobile_state
enable_sync_pinned
enable_text_comment
enable_text_file_cache
enable_thumb_anim_mode
enable_thumb_file_jxl
enable_todo
enable_topology_affinity
enable_total_src_log
enable_tracking_log
enable_tracking_overflow
enable_transform_non_zcloud_status
enable_trigger_recent_sticker_point_e2ee
enable_typing_broadcast
enable_typing_icon
enable_ui
enable_ui_banner_suggest_sync_revamp
enable_ui_banners_in_transfer_revamp
enable_ui_batching_log
enable_ui_popup_transfer_revamp
enable_undo
enable_unread_v2
enable_unstable_connecting
enable_upcoming_event
enable_upgrade
enable_upload
enable_upload_e2ee
enable_upload_file_w_thumb
enable_upload_original
enable_upload_thumb_for_delay_send
enable_use_native_scan
enable_use_scan_messages_v2
enable_user_agent
enable_ux_confirm_transfer_revamp
enable_ux_enhance
enable_validation_on_runtime
enable_verify_checksum_forward_cross
enable_verify_parse_link_server
enable_video
enable_video_call
enable_video_version_2
enable_view
enable_view_folder_option_for_auto_download
enable_warning_full_disk
enable_warning_stranger
enable_wasm
enable_write_log
enable_yellow_banner
enable_youtube
enable_zent_login_email
enable_zinstant
enable_zip_manager
enable_zrf_upload
```

## Full live config (607 keys)

See `flags_dump.json` → `fullLiveConfig`. Every value below was read from the running app, not guessed from the bundle.

## Separate ConfigService registry (`+uX1`)

These 8 config objects are separate from the main config (they have defaults + live server data):

```
comm_metrics
delete_recall_v2
mark_unread
media_audio
media_files
reaction_v2
roll_msg
zinstant_dynamic_banner
```

## localStorage keys (313)

```
0_1246833989869956844_lastReceiveTs
0_1768991032216561197_lastReceiveTs
0_2321617658012150425_lastReceiveTs
0_2598041923061651631_lastReceiveTs
0_3727151243747244056_lastReceiveTs
0_3993317781184376522_lastReceiveTs
0_404817580047378751_lastReceiveTs
0_4095895632053198198_lastReceiveTs
0_4454083929143369585_lastReceiveTs
0_4889216364592998520_lastReceiveTs
0_5023750193297866774_lastReceiveTs
0_5205148733994747549_lastReceiveTs
0_5252879099609587524_lastReceiveTs
0_535413034503691215_lastReceiveTs
0_6558348936034249378_lastReceiveTs
0_6664190823378032090_lastReceiveTs
0_6709214034398344378_lastReceiveTs
0_7303945860385573885_lastReceiveTs
0_7402308958416858651_lastReceiveTs
0_7410766038050638969_lastReceiveTs
0_7466637983069948735_lastReceiveTs
0_7485921100961920578_lastReceiveTs
0_7746435347027384609_lastReceiveTs
0_7805249015969583088_lastReceiveTs
0_824079569974384334_lastReceiveTs
0_8261789236025878278_lastReceiveTs
0_8361434630348066728_lastReceiveTs
0_8390981758187713747_lastReceiveTs
0_8492881917801175933_lastReceiveTs
0_8518513345771926422_lastReceiveTs
0_8936209232253734306_lastReceiveTs
0_8949863659831360149_lastReceiveTs
0_935403203321447770_lastReceiveTs
0_954830842158317537_lastReceiveTs
0_997327615470790774_lastReceiveTs
0_INIT_FRIEND_BA
0_QM__lastUpdate
0_QM__searchTutorialCount
0_WAIT_FOR_SUBMIT_ACTIONS
0__total_mem_per_fetch
0_ac
0_aco
0_action_log_device_id
0_actr
0_actro
0_actrv2
0_acv2
0_adl_action_log_flush_due
0_alias_key
0_biz_suab
0_call_st_audioInput
0_call_st_audioOutput
0_call_st_autoAudioInput
0_call_st_autoAudioOutput
0_call_st_videoInput
0_cdk
0_cdkl
0_citk
0_cl-c-done
0_cl-enk
0_cl-fenk
0_cl-fvk
0_cl-gp-bn-last-content
0_cl-lni
0_cl-s-config
0_cl-vk
0_cl_u_r
0_comm_onb
0_config_alias
0_config_show_unread_time
0_conv-box_lst_1772792272979:1
0_conv-box_scbst_1775549821868
0_ctt_l_a
0_ctt_l_r
0_data_config_hidden_chat
0_data_pin_hidden_chat
0_dbtimepc
0_did_show_edit_caption_tip
0_displayedForwardMessageTip
0_dmn
0_e2eenable
0_e2eopkcleancheck
0_fas
0_first_login_time
0_fr_req_src
0_fts_created_db_record
0_g1425268856711145062_lastReceiveTs
0_g2046764168357267910_lastReceiveTs
0_g2203970506506091063_lastReceiveTs
0_g2382875918545748656_lastReceiveTs
0_g279239451703103809_lastReceiveTs
0_g4820055408076297285_lastReceiveTs
0_g4832025176206802863_lastReceiveTs
0_g4877048316079234831_lastReceiveTs
0_g5683764713652657217_lastReceiveTs
0_g5755965811011525696_lastReceiveTs
0_g5857174311504554985_lastReceiveTs
0_g6444884535142423505_lastReceiveTs
0_g7076404011660797077_lastReceiveTs
0_g7180938916472485382_lastReceiveTs
0_g7553915318141292149_lastReceiveTs
0_g7615275385883325711_lastReceiveTs
0_g7767274256186774379_lastReceiveTs
0_g7858870297461641137_lastReceiveTs
0_grv
0_h_n_stck
0_hide-fbadge-bsha-fb-cbi
0_hide-fbadge-ttl-cb
0_init_fetch_fr
0_is_first_login
0_iurc
0_key_last_fetch_group
0_l_r_m_i::-1
0_l_r_msg
0_last-time-renew-session
0_lastFetchIdKey
0_lastLogSidebarState
0_last_ack_evict
0_last_check_exceed_quota
0_last_time_req_to_me
0_last_time_verify_fr_list
0_lastfilepath
0_lbl-info
0_list_recent_g_search_v2
0_list_recent_search
0_lsmsg
0_me_ts
0_mgripck_v5
0_mig_ver_usr
0_muq
0_mvr_capt_tooltip
0_mvr_photo-viewer_size
0_mz_storage
0_nsp
0_off_tip
0_old_structure_cleanup_progress
0_overflowts
0_pktmr
0_preload_cache
0_r_l_b
0_rcdk
0_react
0_recent_catalog
0_recent_save_path
0_remo
0_rerun_download_v2_last_run
0_res_mnt_detail_cache_data
0_res_mnt_login_timestamp
0_restructure-media:log-migration-result
0_restructure-media:log-migration-status
0_right_sb
0_rm_s
0_rs
0_s_qos_stats_l
0_s_stk
0_search_m_prm
0_signal_cur_opid
0_signallastinit
0_signalserverenable
0_signalsubdevices
0_skip_stk_cat
0_sktmig
0_sm_tl
0_sm_v
0_sock_ac
0_sock_aco
0_sock_ctrl
0_sock_ctrl_ac
0_sock_ctrl_aco
0_sock_msg
0_sock_verfy_510_1
0_sock_verfy_511_1
0_ss_meta
0_ss_meta_exp_dttm
0_ss_resc_exp_dttm
0_ss_resc_stt
0_ss_stkusg
0_st-ver
0_start_date_tpath
0_state_alias
0_sufficient_msg_ts
0_sync_download_cleaner_last_run
0_sync_download_qos_stats
0_sync_download_stats_v1
0_text-file-editor-rect
0_time_alias
0_ttl-v1
0_ttl-v1-saved
0_u_act_i4
0_u_l_f
0_unrImsg
0_update_gi
0_upss
0_ver_bl_friend
0_ver_pin
0_ver_sticker_cate_list
0_version_alias
0_wn
0_zAva-obs-daily
0_zAva-renew-track
0_z_chatbg_reset
0_z_contact
0_z_frq_2658266520063504965
0_z_frq_5205148733994747549
0_z_jumpurl
0_z_ml_g1425268856711145062
0_z_ml_g6444884535142423505
0_z_ml_g7858870297461641137
0_z_phonebook_version
0_z_s2_s_b_
0_z_sc
0_z_sendtome
0_z_sendtome_bubbledot
0_z_sendtome_filessent
0_z_stw
0_z_trackfollow_v2
0_z_trackfrecency_v2
0_za_noti_setting_log_act
0_zcloud-promo
0_zklastefid_1768991032216561197
0_zklastefid_2598041923061651631
0_zklastefid_2611262151019178908
0_zklastefid_3727151243747244056
0_zklastefid_3993317781184376522
0_zklastefid_404817580047378751
0_zklastefid_4454083929143369585
0_zklastefid_4889216364592998520
0_zklastefid_5023750193297866774
0_zklastefid_5252879099609587524
0_zklastefid_535413034503691215
0_zklastefid_6557226175919724382
0_zklastefid_6558348936034249378
0_zklastefid_6664190823378032090
0_zklastefid_6709214034398344378
0_zklastefid_7303945860385573885
0_zklastefid_7402308958416858651
0_zklastefid_7410766038050638969
0_zklastefid_7466637983069948735
0_zklastefid_7746435347027384609
0_zklastefid_8390981758187713747
0_zklastefid_8492881917801175933
0_zklastefid_8936209232253734306
0_zklastefid_935403203321447770
0_zklastefid_997327615470790774
0_zklastefid_g1425268856711145062
0_zklastefid_g2046764168357267910
0_zklastefid_g2203970506506091063
0_zklastefid_g2382875918545748656
0_zklastefid_g279239451703103809
0_zklastefid_g4820055408076297285
0_zklastefid_g4832025176206802863
0_zklastefid_g5755965811011525696
0_zklastefid_g5857174311504554985
0_zklastefid_g6444884535142423505
0_zklastefid_g7076404011660797077
0_zklastefid_g7180938916472485382
0_zklastefid_g7553915318141292149
0_zklastefid_g7767274256186774379
0_zklastefid_g7858870297461641137
0_zpm_m
0_zptd
0_ztbd
0_zum_m
688971448162104366__archived_chat
sh_ADD_SYSTEM_INFO
sh__fetch_kout
sh_app_ver
sh_captcha_cert
sh_codec-detector.h265.collected
sh_config_new_remember
sh_config_opt_remember
sh_d-bu-et
sh_disaster_recovery_data
sh_e1cfeb1a59820a1ef96743cac2394fc4
sh_gr_topics__pin_limit
sh_language
sh_last_time_active_tracking
sh_last_timeonapp_submit
sh_mig_ver_sh
sh_need_set_remember
sh_qos_lastRequestId
sh_qos_url
sh_remme
sh_reset__zklastefid
sh_resource_management_last_time_send_log_disk_status_auto
sh_resource_management_last_time_send_log_migrate_db_auto
sh_s_ph_if
sh_server_maintenance_data
sh_set_value_action_log
sh_sock_cnted
sh_transfer_autodl_cf
sh_user_ids
sh_viewerkey
sh_z_d_temp_last_cleanup_time
sh_z_loginversion
sh_z_needSyncMsg
sh_z_nom
sh_z_recentin
sh_z_recentp
sh_z_recentuid
sh_z_reclogtype
sh_z_recoun
sh_z_uuid
sh_zalo_temp_swap_path_last_time_send_log_report_auto
sh_zcloud_banner_config
sh_zinsrc_26.8.20
sh_zipKey
sh_zlast_uid
sh_zpw_type
sh_zpw_ver
z_uuid
za_theme_log_act
zalo-blur-state-v2
```

## Files

- `flags_dump.json` — complete machine-readable dump.
- `flags_dump.md` — this report.
