/**
 * @file RTE_LOG.c
 * @author Leon Shan (813475603@qq.com)
 * @brief
 * @version 0.1
 * @date 2019-10-27
 *
 * @copyright Copyright (c) 2019
 *
 */
#include "../include/RTE_Log.h"
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <stdlib.h>
/**
 * @brief
 *
 */
typedef struct _log_level_tbl_t {
    log_level_t level;
    char *name;
    char *color;
} log_level_tbl_t;
static const log_level_tbl_t log_level_tbl[] = {
    {LOG_LEVEL_FATAL, "F", COL_FATAL},
    {LOG_LEVEL_ERROR, "E", COL_ERROR},
    {LOG_LEVEL_INFO, "I", COL_INFO},
    {LOG_LEVEL_WARN, "W", COL_WARN},
    {LOG_LEVEL_DEBUG, "D", COL_DEBUG},
    {LOG_LEVEL_VERBOSE, "V", COL_VERBOSE},
};
static log_config_t log_config_handle = {
    .enable = true,
    .level = LOG_LEVEL_WARN,
    .format = LOG_FMT_DEFAULT,
    .filter_cnt = 0,
    .filter_tbl = {
        NULL
    },
    .mutex = NULL,
    .out_func = NULL,
    .mutex_lock_func = NULL,
    .mutex_unlock_func = NULL,
};
int8_t log_init(void *mutex, log_output_f out_func,
                        mutex_lock_f mutex_lock_func,
                        mutex_unlock_f mutex_unlock_func,
                        log_get_tick_f get_tick_func)
{
    int8_t retval = 0;
    log_config_handle.mutex = mutex;
    log_config_handle.out_func = out_func;
    log_config_handle.mutex_lock_func = mutex_lock_func ? mutex_lock_func : log_default_mutex_lock;
    log_config_handle.mutex_unlock_func = mutex_unlock_func ? mutex_unlock_func : log_default_mutex_unlock;
    log_config_handle.get_tick_func = get_tick_func;
    return retval;
}

int8_t log_change_outf(log_output_f out_func)
{
    int8_t retval = 0;
    LOG_LOCK(&log_config_handle);
    log_config_handle.out_func = out_func;
    LOG_UNLOCK(&log_config_handle);
    return retval;
}

static int log_vsnprintf(char * restrict s, size_t n,
                    const char * restrict format,
                    va_list arg)
{
    // A mapping from an integer between 0 and 15 to its ASCII character
    // equivalent.
    static const char * const g_pcHex = "0123456789abcdef";
    unsigned long ulIdx, ulValue, ulCount, ulBase, ulNeg;
    char *pcStr, cFill;
    int iConvertCount = 0;
    // Adjust buffer size limit to allow one space for null termination.
    if(n)
        n--;
    // Initialize the count of characters converted.
    iConvertCount = 0;
    // Loop while there are more characters in the format string.
    while(*format) {
        // Find the first non-% character, or the end of the string.
        for(ulIdx = 0; (format[ulIdx] != '%') && (format[ulIdx] != '\0');
            ulIdx++) {
        }
        // Write this portion of the string to the output buffer.  If there are
        // more characters to write than there is space in the buffer, then
        // only write as much as will fit in the buffer.
        if(ulIdx > n) {
            strncpy(s, format, n);
            s += n;
            n = 0;
        } else {
            strncpy(s, format, ulIdx);
            s += ulIdx;
            n -= ulIdx;
        }
        // Update the conversion count.  This will be the number of characters
        // that should have been written, even if there was not room in the
        // buffer.
        iConvertCount += ulIdx;
        // Skip the portion of the format string that was written.
        format += ulIdx;
        // See if the next character is a %.
        if(*format == '%') {
            // Skip the %.
            format++;
            // Set the digit count to zero, and the fill character to space
            // (that is, to the defaults).
            ulCount = 0;
            cFill = ' ';
            // It may be necessary to get back here to process more characters.
            // Goto's aren't pretty, but effective.  I feel extremely dirty for
            // using not one but two of the beasts.
again:
            // Determine how to handle the next character.
            switch(*format++) {
            // Handle the digit characters.
            case '0':
            case '1':
            case '2':
            case '3':
            case '4':
            case '5':
            case '6':
            case '7':
            case '8':
            case '9': {
                // If this is a zero, and it is the first digit, then the
                // fill character is a zero instead of a space.
                if((format[-1] == '0') && (ulCount == 0))
                    cFill = '0';
                // Update the digit count.
                ulCount *= 10;
                ulCount += format[-1] - '0';
                // Get the next character.
                goto again;
            }
            // Handle the %c command.
            case 'c': {
                // Get the value from the varargs.
                ulValue = va_arg(arg, unsigned long);
                // Copy the character to the output buffer, if there is
                // room.  Update the buffer size remaining.
                if(n != 0) {
                    *s++ = (char)ulValue;
                    n--;
                }
                // Update the conversion count.
                iConvertCount++;
                // This command has been handled.
                break;
            }
            // Handle the %d and %i commands.
            case 'd':
            case 'i': {
                // Get the value from the varargs.
                ulValue = va_arg(arg, unsigned long);
                // If the value is negative, make it positive and indicate
                // that a minus sign is needed.
                if((long)ulValue < 0) {
                    // Make the value positive.
                    ulValue = -(long)ulValue;
                    // Indicate that the value is negative.
                    ulNeg = 1;
                } else {
                    // Indicate that the value is positive so that a
                    // negative sign isn't inserted.
                    ulNeg = 0;
                }
                // Set the base to 10.
                ulBase = 10;
                // Convert the value to ASCII.
                goto convert;
            }
            // Handle the %s command.
            case 's': {
                // Get the string pointer from the varargs.
                pcStr = va_arg(arg, char *);
                // Determine the length of the string.
                for(ulIdx = 0; pcStr[ulIdx] != '\0'; ulIdx++) {
                }
                // Update the convert count to include any padding that
                // should be necessary (regardless of whether we have space
                // to write it or not).
                if(ulCount > ulIdx)
                    iConvertCount += (ulCount - ulIdx);
                // Copy the string to the output buffer.  Only copy as much
                // as will fit in the buffer.  Update the output buffer
                // pointer and the space remaining.
                if(ulIdx > n) {
                    strncpy(s, pcStr, n);
                    s += n;
                    n = 0;
                } else {
                    strncpy(s, pcStr, ulIdx);
                    s += ulIdx;
                    n -= ulIdx;
                    // Write any required padding spaces assuming there is
                    // still space in the buffer.
                    if(ulCount > ulIdx) {
                        ulCount -= ulIdx;
                        if(ulCount > n)
                            ulCount = n;
                        n = -ulCount;
                        while(ulCount--)
                            *s++ = ' ';
                    }
                }
                // Update the conversion count.  This will be the number of
                // characters that should have been written, even if there
                // was not room in the buffer.
                iConvertCount += ulIdx;
                // This command has been handled.
                break;
            }
            // Handle the %u command.
            case 'u': {
                // Get the value from the varargs.
                ulValue = va_arg(arg, unsigned long);
                // Set the base to 10.
                ulBase = 10;
                // Indicate that the value is positive so that a minus sign
                // isn't inserted.
                ulNeg = 0;
                // Convert the value to ASCII.
                goto convert;
            }
            // Handle the %x and %X commands.  Note that they are treated
            // identically; that is, %X will use lower case letters for a-f
            // instead of the upper case letters is should use.  We also
            // alias %p to %x.
            case 'x':
            case 'X':
            case 'p': {
                // Get the value from the varargs.
                ulValue = va_arg(arg, unsigned long);
                // Set the base to 16.
                ulBase = 16;
                // Indicate that the value is positive so that a minus sign
                // isn't inserted.
                ulNeg = 0;
                // Determine the number of digits in the string version of
                // the value.
convert:
                for(ulIdx = 1;
                    (((ulIdx * ulBase) <= ulValue) &&
                        (((ulIdx * ulBase) / ulBase) == ulIdx));
                    ulIdx *= ulBase, ulCount--) {
                }
                // If the value is negative, reduce the count of padding
                // characters needed.
                if(ulNeg)
                    ulCount--;
                // If the value is negative and the value is padded with
                // zeros, then place the minus sign before the padding.
                if(ulNeg && (n != 0) && (cFill == '0')) {
                    // Place the minus sign in the output buffer.
                    *s++ = '-';
                    n--;
                    // Update the conversion count.
                    iConvertCount++;
                    // The minus sign has been placed, so turn off the
                    // negative flag.
                    ulNeg = 0;
                }
                // See if there are more characters in the specified field
                // width than there are in the conversion of this value.
                if((ulCount > 1) && (ulCount < 65536)) {
                    // Loop through the required padding characters.
                    for(ulCount--; ulCount; ulCount--) {
                        // Copy the character to the output buffer if there
                        // is room.
                        if(n != 0) {
                            *s++ = cFill;
                            n--;
                        }
                        // Update the conversion count.
                        iConvertCount++;
                    }
                }
                // If the value is negative, then place the minus sign
                // before the number.
                if(ulNeg && (n != 0)) {
                    // Place the minus sign in the output buffer.
                    *s++ = '-';
                    n--;
                    // Update the conversion count.
                    iConvertCount++;
                }
                // Convert the value into a string.
                for(; ulIdx; ulIdx /= ulBase) {
                    // Copy the character to the output buffer if there is
                    // room.
                    if(n != 0) {
                        *s++ = g_pcHex[(ulValue / ulIdx) % ulBase];
                        n--;
                    }
                    // Update the conversion count.
                    iConvertCount++;
                }
                // This command has been handled.
                break;
            }
            // Handle the %% command.
            case '%': {
                // Simply write a single %.
                if(n != 0) {
                    *s++ = format[-1];
                    n--;
                }
                // Update the conversion count.
                iConvertCount++;
                // This command has been handled.
                break;
            }
            // Handle all other commands.
            default: {
                // Indicate an error.
                if(n >= 5) {
                    strncpy(s, "ERROR", 5);
                    s += 5;
                    n -= 5;
                } else {
                    strncpy(s, "ERROR", n);
                    s += n;
                    n = 0;
                }
                // Update the conversion count.
                iConvertCount += 5;
                // This command has been handled.
                break;
            }
            }
        }
    }
    // Null terminate the string in the buffer.
    *s = 0;
    // Return the number of characters in the full converted string.
    return(iConvertCount);
}

int log_sprintf(char * restrict s, const char *format, ...)
{
    va_list arg;
    int ret;
    // Start the varargs processing.
    va_start(arg, format);
    // Call vsnprintf to perform the conversion.  Use a large number for the
    // buffer size.
    ret = log_vsnprintf(s, 0xffff, format, arg);
    // End the varargs processing.
    va_end(arg);
    // Return the conversion count.
    return(ret);
}

int log_snprintf(char * restrict s, size_t n, const char * restrict format, ...)
{
    va_list arg;
    int ret;
    // Start the varargs processing.
    va_start(arg, format);
    // Call vsnprintf to perform the conversion.
    ret = log_vsnprintf(s, n, format, arg);
    // End the varargs processing.
    va_end(arg);
    // Return the conversion count.
    return(ret);
}

int log_vprintf(const char *pcString, va_list vaArgP)
{
    int iConvertCount = 0;
    uint32_t ui32Idx, ui32Value, ui32Pos, ui32Count, ui32Base, ui32Neg;
    static const char * const g_pcHex = "0123456789abcdef";
    char *pcStr, pcBuf[16], cFill;
    // Loop while there are more characters in the string.
    while(*pcString) {
        // Find the first non-% character, or the end of the string.
        for(ui32Idx = 0;
            (pcString[ui32Idx] != '%') && (pcString[ui32Idx] != '\0');
            ui32Idx++) {
        }
        // Write this portion of the string.
        log_config_handle.out_func((uint8_t *)pcString, ui32Idx);
        iConvertCount += ui32Idx;
        // Skip the portion of the string that was written.
        pcString += ui32Idx;
        // See if the next character is a %.
        if(*pcString == '%') {
            // Skip the %.
            pcString++;
            // Set the digit count to zero, and the fill character to space
            // (in other words, to the defaults).
            ui32Count = 0;
            cFill = ' ';
            // It may be necessary to get back here to process more characters.
            // Goto's aren't pretty, but effective.  I feel extremely dirty for
            // using not one but two of the beasts.
again:
            // Determine how to handle the next character.
            switch(*pcString++) {
                // Handle the digit characters.
                case '0':
                case '1':
                case '2':
                case '3':
                case '4':
                case '5':
                case '6':
                case '7':
                case '8':
                case '9': {
                    // If this is a zero, and it is the first digit, then the
                    // fill character is a zero instead of a space.
                    if((pcString[-1] == '0') && (ui32Count == 0)) {
                        cFill = '0';
                    }
                    // Update the digit count.
                    ui32Count *= 10;
                    ui32Count += pcString[-1] - '0';
                    // Get the next character.
                    goto again;
                }
                // Handle the %c command.
                case 'c': {
                    // Get the value from the varargs.
                    ui32Value = va_arg(vaArgP, uint32_t);
                    // Print out the character.
                    log_config_handle.out_func((uint8_t *)&ui32Value, 1);
                    iConvertCount += 1;
                    // This command has been handled.
                    break;
                }
                // Handle the %d and %i commands.
                case 'd':
                case 'i': {
                    // Get the value from the varargs.
                    ui32Value = va_arg(vaArgP, uint32_t);
                    // Reset the buffer position.
                    ui32Pos = 0;
                    // If the value is negative, make it positive and indicate
                    // that a minus sign is needed.
                    if((int32_t)ui32Value < 0) {
                        // Make the value positive.
                        ui32Value = -(int32_t)ui32Value;
                        // Indicate that the value is negative.
                        ui32Neg = 1;
                    } else {
                        // Indicate that the value is positive so that a minus
                        // sign isn't inserted.
                        ui32Neg = 0;
                    }
                    // Set the base to 10.
                    ui32Base = 10;
                    // Convert the value to ASCII.
                    goto convert;
                }
                // Handle the %s command.
                case 's': {
                    // Get the string pointer from the varargs.
                    pcStr = va_arg(vaArgP, char *);
                    // Determine the length of the string.
                    for(ui32Idx = 0; pcStr[ui32Idx] != '\0'; ui32Idx++) {
                    }
                    // Write the string.
                    log_config_handle.out_func((uint8_t *)pcStr, ui32Idx);
                    // Write any required padding spaces
                    if(ui32Count > ui32Idx) {
                        ui32Count -= ui32Idx;
                        while(ui32Count--) {
                            log_config_handle.out_func((uint8_t *)" ", 1);
                        }
                    }
                    iConvertCount += ui32Idx;
                    // This command has been handled.
                    break;
                }
                // Handle the %u command.
                case 'u': {
                    // Get the value from the varargs.
                    ui32Value = va_arg(vaArgP, uint32_t);
                    // Reset the buffer position.
                    ui32Pos = 0;
                    // Set the base to 10.
                    ui32Base = 10;
                    // Indicate that the value is positive so that a minus sign
                    // isn't inserted.
                    ui32Neg = 0;
                    // Convert the value to ASCII.
                    goto convert;
                }
                // Handle the %x and %X commands.  Note that they are treated
                // identically; in other words, %X will use lower case letters
                // for a-f instead of the upper case letters it should use.  We
                // also alias %p to %x.
                case 'x':
                case 'X':
                case 'p': {
                    // Get the value from the varargs.
                    ui32Value = va_arg(vaArgP, uint32_t);
                    // Reset the buffer position.
                    ui32Pos = 0;
                    // Set the base to 16.
                    ui32Base = 16;
                    // Indicate that the value is positive so that a minus sign
                    // isn't inserted.
                    ui32Neg = 0;
                    // Determine the number of digits in the string version of
                    // the value.
convert:
                    for(ui32Idx = 1;
                        (((ui32Idx * ui32Base) <= ui32Value) &&
                         (((ui32Idx * ui32Base) / ui32Base) == ui32Idx));
                        ui32Idx *= ui32Base, ui32Count--) {
                    }
                    // If the value is negative, reduce the count of padding
                    // characters needed.
                    if(ui32Neg)
                        ui32Count--;
                    // If the value is negative and the value is padded with
                    // zeros, then place the minus sign before the padding.
                    if(ui32Neg && (cFill == '0')) {
                        // Place the minus sign in the output buffer.
                        pcBuf[ui32Pos++] = '-';
                        // The minus sign has been placed, so turn off the
                        // negative flag.
                        ui32Neg = 0;
                    }
                    // Provide additional padding at the beginning of the
                    // string conversion if needed.
                    if((ui32Count > 1) && (ui32Count < 16)) {
                        for(ui32Count--; ui32Count; ui32Count--) {
                            pcBuf[ui32Pos++] = cFill;
                        }
                    }
                    // If the value is negative, then place the minus sign
                    // before the number.
                    if(ui32Neg) {
                        // Place the minus sign in the output buffer.
                        pcBuf[ui32Pos++] = '-';
                    }
                    // Convert the value into a string.
                    for(; ui32Idx; ui32Idx /= ui32Base) {
                        pcBuf[ui32Pos++] =
                            g_pcHex[(ui32Value / ui32Idx) % ui32Base];
                    }
                    // Write the string.
                    log_config_handle.out_func((uint8_t *)pcBuf, ui32Pos);
                    iConvertCount += ui32Pos;
                    // This command has been handled.
                    break;
                }
                // Handle the %% command.
                case '%': {
                    // Simply write a single %.
                    log_config_handle.out_func((uint8_t *)pcString - 1, 1);
                    iConvertCount += 1;
                    // This command has been handled.
                    break;
                }
                // Handle all other commands.
                default: {
                    // Indicate an error.
                    log_config_handle.out_func((uint8_t *)"ERROR", 5);
                    iConvertCount += 5;
                    // This command has been handled.
                    break;
                }
            }
        }
    }
    return iConvertCount;
}

void log_sec2time(time_t timer, struct tm *tm)
{
    // This array contains the number of days in a year at the beginning of each
    // month of the year, in a non-leap year.
    static const time_t g_psDaysToMonth[12] = {
        0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334
    };
    time_t temp, months;
    // Extract the number of seconds, converting time to the number of minutes.
    temp = timer / 60;
    tm->tm_sec = timer - (temp * 60);
    timer = temp;
    // Extract the number of minutes, converting time to the number of hours.
    temp = timer / 60;
    tm->tm_min = timer - (temp * 60);
    timer = temp;
    // Extract the number of hours, converting time to the number of days.
    temp = timer / 24;
    tm->tm_hour = timer - (temp * 24);
    timer = temp;
    // Compute the day of the week.
    tm->tm_wday = (timer + 4) % 7;
    // Compute the number of leap years that have occurred since 1968, the
    // first leap year before 1970.  For the beginning of a leap year, cut the
    // month loop below at March so that the leap day is classified as February
    // 29 followed by March 1, instead of March 1 followed by another March 1.
    timer += 366 + 365;
    temp = timer / ((4 * 365) + 1);
    if((timer - (temp * ((4 * 365) + 1))) > (31 + 28)) {
        temp++;
        months = 12;
    } else {
        months = 2;
    }
    // Extract the year.
    tm->tm_year = ((timer - temp) / 365) + 68;
    timer -= ((tm->tm_year - 68) * 365) + temp;
    // Extract the month.
    for(temp = 0; temp < months; temp++) {
        if(g_psDaysToMonth[temp] > timer)
            break;
    }
    tm->tm_mon = temp - 1;
    // Extract the day of the month.
    tm->tm_mday = timer - g_psDaysToMonth[temp - 1] + 1;
}

size_t log_out(log_level_t level, const char *file, const char *function, int line,
                const char *module, const char *msg, ...)
{
    size_t retval = 0;
    char log_head[LOG_MAX_HEAD_LENGTH] = {0};
    size_t offset = 0;
    uint8_t i = 0;
    LOG_LOCK(&log_config_handle);
    if(!log_config_handle.enable || !log_config_handle.out_func)
        goto end;
    /* Check filter */
    for(i = 0; i < log_config_handle.filter_cnt; i++) {
        if(log_config_handle.filter_tbl[i] && strstr(msg, log_config_handle.filter_tbl[i])) {
            goto end;
        }
    }
    if(level <= log_config_handle.level) {
        if(log_config_handle.format & LOG_FMT_TIME) {
            uint64_t tick = 0;
            struct tm time = {0};
            if(log_config_handle.get_tick_func) {
                tick = log_config_handle.get_tick_func();
            }
            log_sec2time(tick/1000, &time);
            offset += log_snprintf(log_head + offset, LOG_MAX_HEAD_LENGTH - offset, "%02d:%02d:%02d:%03d",
                                    time.tm_hour, time.tm_min, time.tm_sec, tick%1000);
        }
        offset += log_snprintf(log_head + offset, LOG_MAX_HEAD_LENGTH - offset, " %s ",
                                log_level_tbl[level].name);
        log_head[offset] = '[';
        offset++;
        if(log_config_handle.format & LOG_FMT_FILE) {
            offset += log_snprintf(log_head + offset, LOG_MAX_HEAD_LENGTH - offset, "%s", file);
            log_head[offset] = ' ';
            offset++;
        }
        if(log_config_handle.format & LOG_FMT_FUNCTION) {
            offset += log_snprintf(log_head + offset, LOG_MAX_HEAD_LENGTH - offset, "%s", function);
            log_head[offset] = ':';
            offset++;
        }
        if(log_config_handle.format & LOG_FMT_LINE) {
            offset += log_snprintf(log_head + offset, LOG_MAX_HEAD_LENGTH - offset, "%d", line);
        }
        log_head[offset] = ']';
        offset++;
        offset += log_snprintf(log_head + offset, LOG_MAX_HEAD_LENGTH - offset, " %10s: ", module);
        log_config_handle.out_func((uint8_t *)log_head, offset);
        va_list arg;
        va_start(arg, msg);
        offset += log_vprintf(msg, arg);
        va_end(arg);
        log_config_handle.out_func((uint8_t *)"\r\n", 2);
        offset += 2;
        retval += offset;
    }
end:
    LOG_UNLOCK(&log_config_handle);
    return retval;
}

int8_t log_control(log_command_t command, void *param)
{
    int8_t retval = -1;
    uint8_t i = 0;
    LOG_LOCK(&log_config_handle);
    switch(command) {
    case LOG_CMD_ENABLE:
        UNUSED(param);
        log_config_handle.enable = true;
        break;
    case LOG_CMD_DISABLE:
        UNUSED(param);
        log_config_handle.enable = false;
        break;
    case LOG_CMD_LEVEL_UP:
        if(!param)
            break;
        log_config_handle.level++;
        if(log_config_handle.level > LOG_LEVEL_VERBOSE)
            log_config_handle.level = LOG_LEVEL_VERBOSE;
        *((log_level_t *)param) = log_config_handle.level;
        break;
    case LOG_CMD_LEVEL_DOWN:
        if(!param)
            break;
        if(log_config_handle.level >= LOG_LEVEL_ERROR)
            log_config_handle.level--;
        *((log_level_t *)param) = log_config_handle.level;
        break;
    case LOG_CMD_GET_LEVEL:
        if(!param)
            break;
        *((log_level_t *)param) = log_config_handle.level;
        break;
    case LOG_CMD_SET_FORMAT:
        if(!param)
            break;
        log_config_handle.format = *((log_format_t *)param);
        break;
    case LOG_CMD_GET_FORMAT:
        if(!param)
            break;
        *((log_format_t *)param) = log_config_handle.format;
        break;
    case LOG_CMD_ADD_FILTER:
        if(!param)
            break;
        log_config_handle.filter_tbl[log_config_handle.filter_cnt++] = param;
        break;
    case LOG_CMD_RMV_FILTER:
        if(!param)
            break;
        for(i = 0; i < log_config_handle.filter_cnt; i++) {
            if(!strcmp(log_config_handle.filter_tbl[i], (char *)param)) {
                log_config_handle.filter_tbl[i] = NULL;
                memcpy(&log_config_handle.filter_tbl[i], &log_config_handle.filter_tbl[i + 1],
                        (log_config_handle.filter_cnt - i - 1));
                log_config_handle.filter_cnt --;
                break;
            }
        }
        break;
    default:
        break;
    }
    LOG_UNLOCK(&log_config_handle);
    return retval;
}


#if LOG_UST_TEST == 1
#define TEST_MODULE "TEST"
#define TEST_LOGF(...)  LOG_FATAL(TEST_MODULE, __VA_ARGS__)
#define TEST_ASSERT(v)  LOG_ASSERT(TEST_MODULE, v)
static uint64_t test_get_tick(void)
{
    return 123456789;
}
static size_t test_data_out (uint8_t *data,size_t length)
{
    printf("%.*s",length,(char *)data);
    return length;
}
int main(int argc, char *args[])
{
    uint8_t *test_ptr = NULL;
    log_init(NULL, test_data_out, NULL, NULL, test_get_tick);
    TEST_LOGF("Helloworld!");
    TEST_ASSERT(test_ptr);
    return 0;
}
#endif