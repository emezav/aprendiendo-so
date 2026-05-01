/**
 * @file
 * @ingroup kernel_code
 * @brief Cargador minimo de programas ELF desde ext2 hacia modo de usuario.
 */

#ifndef USEREXEC_H_
#define USEREXEC_H_

void setup_user_exec(void);
void register_user_exec_monitor_commands(void);

#endif /* USEREXEC_H_ */
