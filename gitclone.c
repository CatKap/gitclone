#include "libgit2/include/git2/errors.h"
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
        " Received %u/%u objects (%u/%u deltas), %zu bytes\r",
        stats->received_objects,
        stats->total_objects,
        stats->indexed_deltas,
        stats->total_deltas,
        stats->received_bytes
    );
    return 0;
}

int fast_forward(git_repository *repo)
{
    int r;
    git_reference *head = NULL;
    git_reference *remote_ref = NULL;
    git_object *target = NULL;

    /* get repo head */
    r = git_repository_head(&head, repo);
    if (r < 0)
        goto cleanup;

    const char *branch = git_reference_shorthand(head);

    /* refs/remotes/origin/<branch> */
    char remote_ref_name[256];
    snprintf(remote_ref_name, sizeof(remote_ref_name),
             "refs/remotes/origin/%s", branch);

    r = git_reference_lookup(&remote_ref, repo, remote_ref_name);
    if (r < 0)
        goto cleanup;

    /* get object commit */
    r = git_reference_peel(&target, remote_ref, GIT_OBJECT_COMMIT);
    if (r < 0)
        goto cleanup;

    /* checkout on upstream commit */
    git_checkout_options checkout_opts = GIT_CHECKOUT_OPTIONS_INIT;
    checkout_opts.checkout_strategy =
        GIT_CHECKOUT_FORCE | GIT_CHECKOUT_RECREATE_MISSING;

    r = git_checkout_tree(repo, target, &checkout_opts);
    if (r < 0)
        goto cleanup;

    /* move to HEAD */
    r = git_repository_set_head(repo, remote_ref_name);

cleanup:
    git_object_free(target);
    git_reference_free(remote_ref);
    git_reference_free(head);
    return r;
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

    const char* url = argv[1];
    const char* directory = argv[2];
    
    int r = git_clone(&repo, url, directory, &opts);
    if(r == GIT_EEXISTS){
      fprintf(stderr, "Repo exsists, trying to pull...\n");
      
      int error = git_repository_open(&repo, directory);
      if (error < 0) {
        const git_error *e = git_error_last();
        printf("Error %d/%d: %s\n", error, e->klass, e->message);
        exit(error);
      }
      
      r = fast_forward(repo);
    }

    printf("%d\n", r); 
    if (r < 0) {

        const git_error *e = git_error_last();
        fprintf(stderr, "Error: %s\n", e && e->message ? e->message : "unknown\n");
    }

    git_repository_free(repo);
    git_libgit2_shutdown();
    return r;
}
