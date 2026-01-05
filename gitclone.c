#include <git2.h>
#include <stdio.h>


/* SSH credentials callback */
int cred_cb(git_cred **out, const char *url, const char *username_from_url,
            unsigned int allowed_types, void *payload) {
    (void)payload; // unused

    return git_cred_ssh_key_from_agent(out, username_from_url);
}

int main(int argc, char **argv) {
    git_libgit2_init();

    git_repository *repo = NULL;
    git_clone_options opts = GIT_CLONE_OPTIONS_INIT;
    git_fetch_options fetch_opts = GIT_FETCH_OPTIONS_INIT;

    fetch_opts.callbacks.credentials = cred_cb;
    opts.fetch_opts = fetch_opts;

    int r = git_clone(&repo, argv[1], argv[2], &opts);
    if (r < 0) {
        const git_error *e = git_error_last();
        fprintf(stderr, "Error: %s\n", e && e->message ? e->message : "unknown\n");
    }

    git_repository_free(repo);
    git_libgit2_shutdown();
    return r;
}
