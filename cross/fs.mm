#import "cross.hpp"
#import "fs.hpp"
#import "objc.hpp"
#import "logger.hpp"

CROSS_BEGIN

bool mv(string const& from, string const& to)
{
    NSError *err;
    [NSFileManager.defaultManager moveItemAtPath:toOc(from)
                                          toPath:toOc(to)
                                           error:&err];
    if (err) {
        Logger::Error(fromOc(err.description));
        return false;
    }
    return true;
}

static string gs_path_home;

string path_home()
{
    if (gs_path_home.empty()) {
        gs_path_home = fromOc(NSHomeDirectory());
    }
    return gs_path_home;
}

void path_home(string const& v)
{
    gs_path_home = v;
}

static string gs_path_docs;

string path_documents()
{
    if (gs_path_docs.empty()) {
        gs_path_docs = fromOc(NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES).firstObject);
    }
    return gs_path_docs;
}

void path_documents(string const& v)
{
    gs_path_docs = v;
}

static string gs_path_tmp;

string path_tmp()
{
    if (gs_path_tmp.empty()) {
        gs_path_tmp = fromOc(NSTemporaryDirectory());
    }
    return gs_path_tmp;
}

void path_tmp(string const& v)
{
    gs_path_tmp = v;
}

static string gs_path_app;

string path_app()
{
    if (gs_path_app.empty()) {
        gs_path_app = fromOc([[NSBundle mainBundle] bundlePath]);
    }
    return gs_path_app;
}

CROSS_END
