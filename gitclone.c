#include <git2.h>
#include <stdio.h>


/* SSH credentials callback */
int cred_cb(git_cred **out, const char *url, const char *username_from_url,
            unsigned int allowed_types, void *payload) {
    (void)payload; // unused

    return git_cred_ssh_key_from_agent(out, username_from_url);
}

static int transfer_progress_cb(
    const git_indexer_progress *stats,
    void *payload)
{
    fprintf(stderr,
        "Received %u/%u objects (%u/%u deltas), %zu bytes\r",
        stats->received_objects,
        stats->total_objects,
        stats->indexed_deltas,
        stats->total_deltas,
        stats->received_bytes
    );
    return 0;
}

int main(int argc, char **argv) {
    git_libgit2_init();
    if(argc < 3){
      printf("Usage: gitclone [url] [destination dir] {optional:branch}\n");
    return 1;
    }

    const char *branch = (argc >= 4) ? argv[3] : NULL;
    git_repository *repo = NULL;
    git_clone_options opts = GIT_CLONE_OPTIONS_INIT;
    git_fetch_options fetch_opts = GIT_FETCH_OPTIONS_INIT;
    fetch_opts.callbacks.transfer_progress = transfer_progress_cb;

    /* shallow clone */
    fetch_opts.depth = 1;
fetch_opts.callbacks.transfer_progress = transfer_progress_cb;
    fetch_opts.callbacks.credentials = cred_cb;
    opts.fetch_opts = fetch_opts;

    if (branch) {
        printf("Cloning branch %s\n", branch);
        opts.checkout_branch = branch;
    }

    int r = git_clone(&repo, argv[1], argv[2], &opts);
    if (r < 0) {
        const git_error *e = git_error_last();
        fprintf(stderr, "Error: %s\n", e && e->message ? e->message : "unknown\n");
    }

    git_repository_free(repo);
    git_libgit2_shutdown();
    return r;
}
