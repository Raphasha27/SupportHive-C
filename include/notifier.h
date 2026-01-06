#ifndef NOTIFIER_H
#define NOTIFIER_H

void notify_escalation(const char *tenant_id, int ticket_id, int level);
void notify_ticket_created(const char *tenant_id, int ticket_id);

#endif
