#include <netdb.h>
#include <stdlib.h>

/* __BEGIN_DECLS */
#ifdef __cplusplus
extern "C" {
#endif

int getrrsetbyname(const char *hostname, unsigned int rdclass, unsigned int rdtype, unsigned int flags, struct rrsetinfo **res)
{
    // TODO
    return ERRSET_SUCCESS;
}

void freerrset(struct rrsetinfo *rrset)
{
    if(rrset) {
        if(rrset->rri_nrdatas && rrset->rri_rdatas) {
            free(rrset->rri_rdatas);
        }
        if(rrset->rri_nsigs && rrset->rri_sigs) {
            free(rrset->rri_sigs);
        }
        free(rrset);
    }
}

/* __END_DECLS */
#ifdef __cplusplus
}
#endif