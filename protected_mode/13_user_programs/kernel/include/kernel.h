/**
 * @file
 * @brief Interfaz minima del kernel usada entre modulos de `12_user_shell`.
 */

#ifndef KERNEL_H_
#define KERNEL_H_

void kernel_enter_monitor_loop(void);
void kernel_resume_monitor_after_user_exit(void);

#endif /* KERNEL_H_ */
