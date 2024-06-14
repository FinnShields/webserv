/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Response.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: bsyvasal <bsyvasal@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/14 13:05:15 by bsyvasal          #+#    #+#             */
/*   Updated: 2024/06/14 13:35:00 by bsyvasal         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Response.hpp"

void Response::get()
{
	std::cout << "GET method is not supported" << std::endl;
}

void Response::post()
{
	std::cout << "POST method is not supported" << std::endl;
}
void Response::answer(Request &req)
{
	std::string method = req.get("method");
    if (method == "GET")
        get();
    else if (method == "POST")
        post();
}
