#pragma once
#include <iostream>
#include <dsn/utility/filesystem.h>
#include <dsn/dist/block_service.h>
#include <dsn/cpp/auto_codes.h>
#include <dsn/cpp/clientlet.h>
#include "../../../lib/replica_context.h"
#include "replication_service_test_app.h"
using namespace ::dsn;
using namespace ::dsn::dist::block_service;
using namespace ::dsn::replication;

class block_file_mock;

extern ref_ptr<block_file_mock> current_chkpt_file;
extern ref_ptr<block_file_mock> backup_metadata_file;
extern ref_ptr<block_file_mock> regular_file;
extern ref_ptr<block_file_mock> not_exist_file;

class block_file_mock : public block_file
{
public:
    block_file_mock(const std::string &_name, int64_t _size, const std::string &_md5)
        : block_file(_name),
          size(_size),
          md5(_md5),
          enable_write_fail(false),
          enable_read_fail(false),
          enable_upload_fail(false)
    {
    }

    virtual uint64_t get_size() { return static_cast<uint64_t>(size); }

    virtual const std::string &get_md5sum() { return md5; }

    virtual dsn::task_ptr write(const write_request &req,
                                dsn::task_code code,
                                const write_callback &cb,
                                clientlet *tracker = nullptr)
    {
        write_response resp;
        if (enable_write_fail) {
            resp.err = ERR_MOCK_INTERNAL;
        } else {
            resp.err = ERR_OK;
            context = std::move(req.buffer);
            resp.written_size = context.length();
        }
        cb(resp);
        return task_ptr();
    }

    virtual dsn::task_ptr read(const read_request &req,
                               dsn::task_code code,
                               const read_callback &cb,
                               clientlet *tracker = nullptr)
    {
        read_response resp;
        if (enable_read_fail) {
            resp.err = ERR_MOCK_INTERNAL;
        } else {
            resp.err = ERR_OK;
            if (size <= 0 && md5.empty()) {
                resp.buffer = blob();
            } else {
                resp.buffer = context;
            }
        }
        cb(resp);
        return task_ptr();
    }

    virtual dsn::task_ptr upload(const upload_request &req,
                                 dsn::task_code code,
                                 const upload_callback &cb,
                                 clientlet *tracker = nullptr)
    {
        upload_response resp;
        if (enable_upload_fail) {
            resp.err = ERR_MOCK_INTERNAL;
        } else {
            resp.err = ERR_OK;
            // just return the file size
            resp.uploaded_size = size;
        }
        cb(resp);
        return task_ptr();
    }

    virtual dsn::task_ptr download(const download_request &req,
                                   dsn::task_code code,
                                   const download_callback &cb,
                                   clientlet *tracker = nullptr)
    {
        return task_ptr();
    }

    // make file exist,
    void file_exist(const std::string &_md5, int64_t _size)
    {
        md5 = _md5;
        size = _size;
    }
    // make file not exist
    void clear_file_exist()
    {
        size = 0;
        md5.clear();
    }

    void set_context(const std::string &value)
    {
        auto len = value.length();
        std::shared_ptr<char> buf = utils::make_shared_array<char>(len);
        ::memcpy(buf.get(), value.c_str(), len);
        blob write_buf(std::move(buf), static_cast<unsigned int>(len));
        context = std::move(write_buf);
    }
    void clear_context() { context = blob(); }

public:
    int64_t size;
    std::string md5;
    blob context;
    bool enable_write_fail;
    bool enable_read_fail;
    bool enable_upload_fail;
};

class block_service_mock : public block_filesystem
{
public:
    block_service_mock()
        : block_filesystem(), enable_create_file_fail(false), enable_list_dir_fail(false)
    {
    }
    virtual error_code initialize(const std::vector<std::string> &args) { return ERR_OK; }

    virtual dsn::task_ptr list_dir(const ls_request &req,
                                   dsn::task_code code,
                                   const ls_callback &callback,
                                   clientlet *tracker = nullptr)
    {
        ls_response resp;
        if (enable_list_dir_fail) {
            resp.err = ERR_MOCK_INTERNAL;
        } else {
            resp.err = ERR_OK;
            std::string dir_name = ::dsn::utils::filesystem::get_file_name(req.dir_name);
            if (dir_files.find(dir_name) != dir_files.end()) {
                resp.entries = std::make_shared<std::vector<ls_entry>>();
                (*resp.entries) = dir_files[dir_name];
            } else {
                resp.err = ERR_OBJECT_NOT_FOUND;
            }
        }
        callback(resp);
        return task_ptr();
    }

    virtual dsn::task_ptr create_file(const create_file_request &req,
                                      dsn::task_code code,
                                      const create_file_callback &cb,
                                      clientlet *tracker = nullptr)
    {
        create_file_response resp;
        if (enable_create_file_fail) {
            resp.err = ERR_MOCK_INTERNAL;
        } else {
            resp.err = ERR_OK;
            auto it = files.find(req.file_name);
            if (it != files.end()) {
                resp.file_handle =
                    new block_file_mock(req.file_name, it->second.first, it->second.second);
            } else {
                std::string filename = ::dsn::utils::filesystem::get_file_name(req.file_name);
                if (filename == cold_backup_constant::CURRENT_CHECKPOINT) {
                    resp.file_handle = current_chkpt_file;
                    std::cout << "current_ckpt_file is selected..." << std::endl;
                } else if (filename == cold_backup_constant::BACKUP_METADATA) {
                    resp.file_handle = backup_metadata_file;
                    std::cout << "backup_metadata_file is selected..." << std::endl;
                } else {
                    resp.file_handle = regular_file;
                    std::cout << "regular_file is selected..." << std::endl;
                }
            }
        }

        cb(resp);
        return task_ptr();
    }

    virtual dsn::task_ptr delete_file(const delete_file_request &req,
                                      dsn::task_code code,
                                      const delete_file_callback &cb,
                                      clientlet *tracker = nullptr)
    {
        return task_ptr();
    }

    dsn::task_ptr remove_path(const remove_path_request &req,
                              dsn::task_code code,
                              const remove_path_callback &cb,
                              clientlet *tracker)
    {
        return task_ptr();
    }

    dsn::task_ptr exist(const exist_request &req,
                        dsn::task_code code,
                        const exist_callback &cb,
                        clientlet *tracker)
    {
        return task_ptr();
    }

public:
    std::map<std::string, std::vector<ls_entry>> dir_files;
    std::map<std::string, std::pair<int64_t, std::string>> files;
    bool enable_create_file_fail;
    bool enable_list_dir_fail;
};
