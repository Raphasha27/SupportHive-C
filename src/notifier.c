#include <stdio.h>
#include "notifier.h"

void notify_escalation(const char *tenant_id, int ticket_id, int level) {
    printf("\n--- [NOTIFICATION SENT] ---\n");
    printf("To: Management Team (%s)\n", tenant_id);
    printf("Subject: 🚩 URGENT: SLA Breach for Ticket #%d\n", ticket_id);
    printf("Message: Ticket #%d has exceeded its SLA threshold. Escalated to Level %d.\n", ticket_id, level);
    printf("---------------------------\n\n");
}

void notify_ticket_created(const char *tenant_id, int ticket_id) {
    printf("\n--- [NOTIFICATION SENT] ---\n");
    printf("To: Support Staff (%s)\n", tenant_id);
    printf("Subject: New Ticket Created #%d\n", ticket_id);
    printf("---------------------------\n\n");
}
