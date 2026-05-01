/**
 * @file
 * @ingroup kernel_code
 * @author Erwin Meza <emezav@gmail.com>
 * @copyright GNU Public License.
 *
 * @brief Primitivas de observabilidad dual para consola y puerto serial.
 */

#ifndef KLOG_H_
#define KLOG_H_

/**
 * @brief Imprime un mensaje tanto en la consola VGA como en el puerto serial.
 * @param format Cadena de formato.
 * @param ... Argumentos asociados al formato.
 */
void klog_printf(char * format, ...);

#endif /* KLOG_H_ */
